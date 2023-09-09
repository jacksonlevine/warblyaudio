#include <portaudio.h>
#include <iostream>
#include <vector>
#include <SFML/Audio.hpp>

const int SAMPLE_RATE = 1200;
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
    if(isRecording) {
        // Record audio data
        for (int i = 0; i < frameCount * NUM_CHANNELS; ++i) {
            data->recordedSamples.push_back(input[i]);
        }
    }
    // Playback audio data (passthrough)
    for (int i = 0; i < frameCount * NUM_CHANNELS; ++i) {
        output[i] = input[i];
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

    err = Pa_OpenDefaultStream(&stream, NUM_CHANNELS, NUM_CHANNELS, paFloat32, SAMPLE_RATE, FRAMES_PER_BUFFER, audioCallback, &userData);
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

    std::vector<sf::Int16> int16Samples;
    for (float sample : userData.recordedSamples) {
        int16Samples.push_back(static_cast<sf::Int16>(sample * 32767.0f)); // Convert to 16-bit
    }

 // Save the recorded audio to a file using SFML
    sf::SoundBuffer recordedBuffer;
    recordedBuffer.loadFromSamples(int16Samples.data(), userData.recordedSamples.size(), NUM_CHANNELS, SAMPLE_RATE);
    recordedBuffer.saveToFile("recorded_audio.wav");


    return 0;
}
