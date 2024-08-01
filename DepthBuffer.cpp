#include "DepthBuffer.h"

#include "ZAssert.h"
#include "PlatformMemory.h"
#include "Delegate.h"
#include "ZConfig.h"
#include "PlatformApplication.h"
#include "PlatformIntrinsics.h"
#include "ScopedTimer.h"

#include <cmath>

namespace ZSharp {

DepthBuffer::DepthBuffer() {
  const ZConfig& config = ZConfig::Get();
  OnResize(config.GetViewportWidth().Value(), config.GetViewportHeight().Value());

  OnWindowSizeChangedDelegate().Add(Delegate<size_t, size_t>::FromMember<DepthBuffer, &DepthBuffer::OnResize>(this));
}

DepthBuffer::~DepthBuffer() {
  if (mData != nullptr) {
    PlatformAlignedFree(mData);
  }

  OnWindowSizeChangedDelegate().Remove(Delegate<size_t, size_t>::FromMember<DepthBuffer, &DepthBuffer::OnResize>(this));
}

float& DepthBuffer::operator[](size_t index) {
  ZAssert(index < (mWidth * mHeight));
  return mData[index];
}

void DepthBuffer::Clear(size_t begin, size_t size) {
  size_t totalSize = mWidth * mHeight * sizeof(float);
  if (begin > totalSize || (begin + size) > totalSize) {
    return;
  }

  const float clearValue = -1.f;

  size_t nearestSize = RoundDownNearestMultiple(size, 4);
  if (nearestSize > 0) {
    Aligned_Memset(((uint8*)mData) + begin, *reinterpret_cast<const uint32*>(&clearValue), nearestSize);
  }

  for (size_t i = nearestSize / 4; i < size / 4; ++i) {
    *(mData + i) = clearValue;
  }
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
  mData = static_cast<float*>(PlatformAlignedMalloc(totalSize, PlatformAlignmentGranularity()));
}

}
