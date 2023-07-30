#pragma once

#include "ZBaseTypes.h"

namespace ZSharp {

struct PlatformAudioDevice;

/*
Initialize an audio device given a sample rate, number of channels, and frame duration.

i.e. for CD quality stereo this is 48000Hz, 2 channel, and 16(ms) for 60FPS.
*/
PlatformAudioDevice* PlatformInitializeAudioDevice(size_t samplesPerSecond, 
  size_t numChannels,
  size_t durationMillisecond);

size_t PlatformPlayAudio(PlatformAudioDevice* device, int16* pcmSignal, size_t offset, size_t endTrack);

void PlatformReleaseAudioDevice(PlatformAudioDevice* device);

}
