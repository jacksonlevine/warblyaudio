# Bad Microphone

Learning the basics of PortAudio, made this simple CLI tool to hear your microphone at a 4200hz sample rate, and record a file (it is exported to 'upsampled_audio.wav' in the current directory)

## Libraries Used:
* SFML to save audio buffers to a file.
* PortAudio to get the default input and stream it at a low sample rate.
* Libsamplerate to upsample to 44100 to be playable in common media players.

Enjoy!
