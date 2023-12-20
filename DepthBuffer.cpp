#include "DepthBuffer.h"

#include "ZAssert.h"
#include "PlatformMemory.h"
#include "Delegate.h"
#include "ZConfig.h"
#include "Win32PlatformApplication.h"
#include "PlatformIntrinsics.h"
#include "ScopedTimer.h"

#include <cmath>

namespace ZSharp {

DepthBuffer::DepthBuffer() {
  const ZConfig& config = ZConfig::Get();
  OnResize(config.GetViewportWidth().Value(), config.GetViewportHeight().Value());

  Win32PlatformApplication::OnWindowSizeChangedDelegate.Add(Delegate<size_t, size_t>::FromMember<DepthBuffer, &DepthBuffer::OnResize>(this));
}

DepthBuffer::~DepthBuffer() {
  if (mData != nullptr) {
    PlatformAlignedFree(mData);
  }

  Win32PlatformApplication::OnWindowSizeChangedDelegate.Remove(Delegate<size_t, size_t>::FromMember<DepthBuffer, &DepthBuffer::OnResize>(this));
}

float& DepthBuffer::operator[](size_t index) {
  ZAssert(index < (mWidth * mHeight));
  return mData[index];
}

void DepthBuffer::Clear() {
  NamedScopedTimer(ClearDepthBuffer);
  const float clearValue = INFINITY;

  Aligned_Memset(mData, *reinterpret_cast<const uint32*>(&clearValue), mWidth * mHeight * sizeof(float));
}

float* DepthBuffer::GetBuffer() {
  return mData;
}

size_t DepthBuffer::GetWidth() const {
  return mWidth;
}

size_t DepthBuffer::GetHeight() const {
  return mHeight;
}

void DepthBuffer::OnResize(size_t width, size_t height) {
  if (mData != nullptr) {
    PlatformAlignedFree(mData);
  }

  mWidth = width;
  mHeight = height;
  const size_t totalSize = width * height * sizeof(float);
  mData = static_cast<float*>(PlatformAlignedMalloc(totalSize, 64));
}

}
