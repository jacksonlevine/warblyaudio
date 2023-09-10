







#include <portaudio.h>
#include <iostream>
#include <vector>
#include <SFML/Audio/SoundBufferRecorder.hpp>
#include <samplerate.h>

const int ORIGINAL_SAMPLE_RATE = 4200;
const int TARGET_SAMPLE_RATE = 44100;
const int FRAMES_PER_BUFFER = 1;
const int NUM_CHANNELS = 2; // Stereo (1 for mono)

struct UserData {
    std::vector<float> recordedSamples;
};

static bool isRecording = false;

int audioCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData) {
    UserData* data = static_cast<UserData*>(userData);
    const float* input = static_cast<const float*>(inputBuffer);
    float* output = static_cast<float*>(outputBuffer);
   

    // Playback audio data (passthrough)
    for (int i = 0; i < frameCount * NUM_CHANNELS; ++i) {
        output[i] = input[i];
    }

     if (isRecording) {
        // Record audio data
        for (int i = 0; i < frameCount * NUM_CHANNELS; ++i) {
            data->recordedSamples.push_back(output[i]);
        }
    }

    return paContinue; // Continue audio processing
}

int main() {
    PaError err;

    err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        return 1;
    }

    UserData userData;
    PaStream* stream;

    err = Pa_OpenDefaultStream(&stream, NUM_CHANNELS, NUM_CHANNELS, paFloat32, ORIGINAL_SAMPLE_RATE, FRAMES_PER_BUFFER, audioCallback, &userData);
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        Pa_Terminate();
        return 1;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        Pa_CloseStream(stream);
        Pa_Terminate();
        return 1;
    }

    std::cout << "Playing audio... Press Enter to start recording." << std::endl;
    std::cin.get(); // Wait for user input

    isRecording = true;

    std::cout << "Recording... Press Enter to stop." << std::endl;
    std::cin.get(); // Wait for user input

    err = Pa_StopStream(stream);
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
    }

    err = Pa_CloseStream(stream);
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
    }

    Pa_Terminate();

    // Upsample the recorded audio data using libsamplerate
    const int inputSampleRate = ORIGINAL_SAMPLE_RATE;
    const int outputSampleRate = TARGET_SAMPLE_RATE;
    const int numChannels = NUM_CHANNELS;

    SRC_STATE* resampler = src_new(SRC_SINC_FASTEST, numChannels, &err);

    if (resampler == nullptr) {
        std::cerr << "libsamplerate error: Failed to create resampler" << std::endl;
        return 1;
    }

    const int inputFrames = static_cast<int>(userData.recordedSamples.size() / numChannels);
    const int outputFrames = static_cast<int>((static_cast<double>(inputFrames) / inputSampleRate) * outputSampleRate);

    std::vector<float> resampledData(outputFrames * numChannels);

    SRC_DATA srcData;
    srcData.data_in = userData.recordedSamples.data();
    srcData.data_out = resampledData.data();
    srcData.input_frames = inputFrames;
    srcData.output_frames = outputFrames;
    srcData.src_ratio = static_cast<double>(outputSampleRate) / inputSampleRate;

    src_process(resampler, &srcData);

    
    // Translate into int16 for SFML
    std::vector<sf::Int16> int16Samples;
    for (float sample : resampledData) {
        int16Samples.push_back(static_cast<sf::Int16>(sample * 32767.0f)); // Convert to 16-bit
    }

    // Save the upsampled audio to a file using SFML
    sf::SoundBuffer recordedBuffer;
    recordedBuffer.loadFromSamples(int16Samples.data(), int16Samples.size(), NUM_CHANNELS, TARGET_SAMPLE_RATE);
    recordedBuffer.saveToFile("upsampled_audio.wav");

    src_delete(resampler);

    return 0;
}

