#pragma once

#include "ZBaseTypes.h"

namespace ZSharp {

struct PlatformAudioDevice;

PlatformAudioDevice* PlatformInitializeAudioDevice();

void PlatformPlayAudio(PlatformAudioDevice* device, int16* pcmSignal, size_t length);

void PlatformReleaseAudioDevice(PlatformAudioDevice* device);

}
