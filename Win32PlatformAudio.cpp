#ifdef PLATFORM_WINDOWS

#include "PlatformAudio.h"

#include <cstring>

#include <AudioSessionTypes.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <ksmedia.h>

#include <initguid.h>

/*
  Windows complains about not being able to find these.
*/
DEFINE_GUID(my_IID_IAudioClient,            0x1CB9AD4C, 0xDBFA, 0x4c32, 0xB1, 0x78, 0xC2, 0xF5, 0x68, 0xA7, 0x03, 0xB2);
DEFINE_GUID(my_IID_IAudioClient2,           0x726778CD, 0xF60A, 0x4eda, 0x82, 0xDE, 0xE4, 0x76, 0x10, 0xCD, 0x78, 0xAA);
DEFINE_GUID(my_IID_IAudioRenderClient,      0xF294ACFC, 0x3146, 0x4483, 0xA7, 0xBF, 0xAD, 0xDC, 0xA7, 0xC2, 0x60, 0xE2);
DEFINE_GUID(my_IID_IAudioClock,             0xCD63314F, 0x3FBA, 0x4a1b, 0x81, 0x2C, 0xEF, 0x96, 0x35, 0x87, 0x28, 0xE7);
DEFINE_GUID(my_IID_IMMDeviceEnumerator,     0xA95664D2, 0x9614, 0x4F35, 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6);
DEFINE_GUID(my_CLSID_MMDeviceEnumerator,    0xBCDE0395, 0xE52F, 0x467C, 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E);

namespace ZSharp {

struct PlatformAudioDevice {
  IMMDevice* deviceHandle = nullptr;
  IAudioClient2* clientHandle = nullptr;
  IAudioRenderClient* renderClientHandle = nullptr;
};

PlatformAudioDevice* PlatformInitializeAudioDevice(size_t samplesPerSecond, size_t numChannels, size_t durationMillisecond) {
  REFERENCE_TIME hnsRequestedDuration = durationMillisecond * 10000; // In 100ns intervals
  IMMDeviceEnumerator* pEnumerator = nullptr;
  IMMDevice* pDevice = nullptr;
  IAudioClient2* pAudioClient = nullptr;
  IAudioRenderClient* pRenderClient = nullptr;

  HRESULT hr = CoCreateInstance(
    my_CLSID_MMDeviceEnumerator, NULL,
    CLSCTX_ALL, my_IID_IMMDeviceEnumerator,
    (void**)&pEnumerator);

  if (hr != S_OK) {
    return nullptr;
  }

  hr = pEnumerator->GetDefaultAudioEndpoint(
    eRender, eConsole, &pDevice);

  if (hr != S_OK) {
    pEnumerator->Release();
    return nullptr;
  }

  hr = pDevice->Activate(
    my_IID_IAudioClient2, CLSCTX_ALL,
    NULL, (void**)&pAudioClient);

  if (hr != S_OK) {
    return nullptr;
  }

  AudioClientProperties properties = {};
  properties.cbSize = sizeof(properties);
  properties.bIsOffload = false;
  properties.eCategory = AudioCategory_GameMedia;
  properties.Options = AUDCLNT_STREAMOPTIONS_RAW;

  hr = pAudioClient->SetClientProperties(&properties);

  if (hr != S_OK) {
    return nullptr;
  }

  REFERENCE_TIME defaultPeriod = {};
  REFERENCE_TIME minPeriod = {};
  hr = pAudioClient->GetDevicePeriod(&defaultPeriod, &minPeriod);

  if (hr != S_OK) {
    return nullptr;
  }

#if 0
  WAVEFORMATEX waveFormat = {};
  waveFormat.wFormatTag = WAVE_FORMAT_PCM;
  waveFormat.nChannels = (WORD)numChannels;
  waveFormat.nSamplesPerSec = (DWORD)samplesPerSecond;
  waveFormat.nAvgBytesPerSec = (DWORD)((samplesPerSecond) * ((numChannels * 16) / 8));
  waveFormat.nBlockAlign = (WORD)((numChannels * 16) / 8);
  waveFormat.wBitsPerSample = 16;
  waveFormat.cbSize = 0;
#endif

  (void)numChannels;
  (void)samplesPerSecond;

  WAVEFORMATEX* actualFormat = nullptr;
  hr = pAudioClient->GetMixFormat(&actualFormat);

  if (hr != S_OK) {
    return 0;
  }

  /*DWORD flags = (AUDCLNT_STREAMFLAGS_RATEADJUST
    | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM
    | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY);*/

  DWORD flags = (AUDCLNT_STREAMFLAGS_RATEADJUST
    | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM
    | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY);

  hr = pAudioClient->Initialize(
    AUDCLNT_SHAREMODE_SHARED,
    flags,
    hnsRequestedDuration,
    //minPeriod,
    0,
    actualFormat,
    NULL);

  if (hr != S_OK) {
    return nullptr;
  }

  hr = pAudioClient->GetService(
    my_IID_IAudioRenderClient,
    (void**)&pRenderClient);

  if (hr != S_OK) {
    return nullptr;
  }

  pEnumerator->Release();

  {
    BYTE* data = nullptr;
    UINT32 bufferSize = 0;
    pAudioClient->GetBufferSize(&bufferSize);

    pRenderClient->GetBuffer(bufferSize, &data);

    memset(data, 0, bufferSize * 8);

    pRenderClient->ReleaseBuffer(bufferSize, AUDCLNT_BUFFERFLAGS_SILENT);
  }

  hr = pAudioClient->Start();

  PlatformAudioDevice* audioHandle = new PlatformAudioDevice;
  audioHandle->deviceHandle = pDevice;
  audioHandle->clientHandle = pAudioClient;
  audioHandle->renderClientHandle = pRenderClient;

  return audioHandle;
}

size_t PlatformPlayAudio(PlatformAudioDevice* device, float* pcmSignal, size_t offset, size_t endTrack, size_t milliseconds) {
  if (device == nullptr ||
    device->deviceHandle == nullptr ||
    device->clientHandle == nullptr ||
    device->renderClientHandle == nullptr) {
    return 0;
  }

  WAVEFORMATEX* actualFormat = nullptr;
  HRESULT hr = device->clientHandle->GetMixFormat(&actualFormat);

  if (hr != S_OK) {
    return 0;
  }

#if 0
  WAVEFORMATEXTENSIBLE* extendedFormat = (WAVEFORMATEXTENSIBLE*)actualFormat;
  (void)extendedFormat;
#endif

  UINT32 bufferFrameCount;
  hr = device->clientHandle->GetBufferSize(&bufferFrameCount);

  if (hr != S_OK) {
    return 0;
  }

  UINT32 numFramesPadding = 0;
  hr = device->clientHandle->GetCurrentPadding(&numFramesPadding);

  if (hr != S_OK) {
    return 0;
  }

  UINT32 numFramesAvailable = bufferFrameCount - numFramesPadding;
  if (numFramesAvailable == 0) {
    return 0;
  }

  const size_t frameSize = (size_t)actualFormat->nBlockAlign;

  const size_t numSamplesToQueue = milliseconds * (actualFormat->nAvgBytesPerSec / 1000);
  if ((numSamplesToQueue / frameSize) > numFramesAvailable) {
    return 0;
  }

  numFramesAvailable = (UINT32)(numSamplesToQueue / frameSize);

  if (offset + (numFramesAvailable * frameSize) > endTrack) {
    numFramesAvailable = (UINT32)((endTrack - offset) / frameSize);
  }

  BYTE* pData;
  hr = device->renderClientHandle->GetBuffer(numFramesAvailable, &pData);

  if (hr != S_OK) {
    return 0;
  }

  /*
  NOTE: In exclusive mode the sound card is looking for IEEE_FLOAT format.
    WASAPI DOES NOT PROVIDE A SAMPLE RATE CONVERTER IN EXCLUSIVE MODE.
    It's up to us to convert ourselves. Luckily our sound card is already defaulting to 48KHz.
    We just need to make sure to give it the right float data.

  NOTE: In shared mode WASAPI will do the sample rate conversion for us.
    WASAPI uses 32-bit float internally during mixing so we might be able to get better latency/perf if we give it the native format.
  */

  memcpy(pData, ((uint8*)pcmSignal) + offset, numFramesAvailable * frameSize);

  hr = device->renderClientHandle->ReleaseBuffer(numFramesAvailable, 0);

  if (hr != S_OK) {
    return 0;
  }

  return numFramesAvailable * frameSize;
}

void PlatformReleaseAudioDevice(PlatformAudioDevice* device) {
  if (device == nullptr ||
    device->deviceHandle == nullptr ||
    device->clientHandle == nullptr ||
    device->renderClientHandle == nullptr) {
    return;
  }

  //device->clientHandle->Stop();

  device->deviceHandle->Release();
  device->deviceHandle = nullptr;
  //device->renderClientHandle->Release();
  device->clientHandle->Release();
  device->clientHandle = nullptr;
  //device->clockHandle->Release();
  delete device;
}

}

#endif
