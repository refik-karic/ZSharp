#ifdef PLATFORM_WINDOWS

#include "PlatformAudio.h"

#include <cstring>

#include <AudioSessionTypes.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>

#include <initguid.h>

/*
  Windows complains about not being able to find these.
*/
DEFINE_GUID(my_IID_IAudioClient,            0x1CB9AD4C, 0xDBFA, 0x4c32, 0xB1, 0x78, 0xC2, 0xF5, 0x68, 0xA7, 0x03, 0xB2);
DEFINE_GUID(my_IID_IAudioRenderClient,      0xF294ACFC, 0x3146, 0x4483, 0xA7, 0xBF, 0xAD, 0xDC, 0xA7, 0xC2, 0x60, 0xE2);
DEFINE_GUID(my_IID_IMMDeviceEnumerator,     0xA95664D2, 0x9614, 0x4F35, 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6);
DEFINE_GUID(my_CLSID_MMDeviceEnumerator,    0xBCDE0395, 0xE52F, 0x467C, 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E);

namespace ZSharp {

struct PlatformAudioDevice {
  IMMDevice* deviceHandle = nullptr;
  IAudioClient* clientHandle = nullptr;
  IAudioRenderClient* renderClientHandle = nullptr;
  WAVEFORMATEX* waveFormat = nullptr;
};

PlatformAudioDevice* PlatformInitializeAudioDevice(size_t samplesPerSecond, size_t numChannels, size_t durationMillisecond) {
  HRESULT hr = S_OK;
  REFERENCE_TIME hnsRequestedDuration = durationMillisecond * 10000; // In 100ns intervals
  IMMDeviceEnumerator* pEnumerator = nullptr;
  IMMDevice* pDevice = nullptr;
  IAudioClient* pAudioClient = nullptr;
  IAudioRenderClient* pRenderClient = nullptr;
  WAVEFORMATEX* pwfx = nullptr;

  hr = CoCreateInstance(
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
    my_IID_IAudioClient, CLSCTX_ALL,
    NULL, (void**)&pAudioClient);

  if (hr != S_OK) {
    return nullptr;
  }

  hr = pAudioClient->GetMixFormat(&pwfx);

  if (hr != S_OK) {
    return nullptr;
  }

  pwfx->wFormatTag = WAVE_FORMAT_PCM;
  pwfx->nChannels = (WORD)numChannels;
  pwfx->nSamplesPerSec = (DWORD)samplesPerSecond;
  pwfx->nAvgBytesPerSec = (DWORD)((samplesPerSecond) * ((numChannels * 16) / 8));
  pwfx->nBlockAlign = (WORD)(numChannels * 16) / 8;
  pwfx->wBitsPerSample = 16;
  pwfx->cbSize = 0;

  /*DWORD flags = (AUDCLNT_STREAMFLAGS_RATEADJUST
    | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM
    | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY);*/
  DWORD flags = AUDCLNT_STREAMFLAGS_RATEADJUST;

  hr = pAudioClient->Initialize(
    AUDCLNT_SHAREMODE_SHARED,
    flags,
    hnsRequestedDuration,
    0,
    pwfx,
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

  hr = pAudioClient->Start();

  PlatformAudioDevice* audioHandle = new PlatformAudioDevice;
  audioHandle->deviceHandle = pDevice;
  audioHandle->clientHandle = pAudioClient;
  audioHandle->renderClientHandle = pRenderClient;
  audioHandle->waveFormat = pwfx;

  return audioHandle;
}

size_t PlatformPlayAudio(PlatformAudioDevice* device, int16* pcmSignal, size_t offset, size_t endTrack) {
  if (device == nullptr ||
    device->deviceHandle == nullptr ||
    device->clientHandle == nullptr ||
    device->renderClientHandle == nullptr) {
    return 0;
  }

  UINT32 bufferFrameCount;

  HRESULT hr = device->clientHandle->GetBufferSize(&bufferFrameCount);

  if (hr != S_OK) {
    return 0;
  }

  UINT32 numFramesPadding;
  hr = device->clientHandle->GetCurrentPadding(&numFramesPadding);

  if (hr != S_OK) {
    return 0;
  }

  UINT32 numFramesAvailable = bufferFrameCount - numFramesPadding;
  if (numFramesAvailable == 0) {
    return 0;
  }

  const size_t frameSize = (size_t)device->waveFormat->nBlockAlign;
  if (offset + (numFramesAvailable * frameSize) > endTrack) {
    numFramesAvailable = (UINT32)((endTrack - offset) / frameSize);
  }

  BYTE* pData;
  hr = device->renderClientHandle->GetBuffer(bufferFrameCount, &pData);

  if (hr != S_OK) {
    return 0;
  }

  memcpy(pData + (numFramesPadding * frameSize), ((int8*)pcmSignal) + offset, numFramesAvailable * frameSize);

  hr = device->renderClientHandle->ReleaseBuffer(bufferFrameCount, 0);

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

  device->clientHandle->Stop();

  device->deviceHandle->Release();
  device->clientHandle->Release();
  device->renderClientHandle->Release();
  delete device;
}

}

#endif
