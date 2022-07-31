#include "Framebuffer.h"

#include <cstdlib>

#include "ZAssert.h"
#include "PlatformMemory.h"
#include "ZConfig.h"

#ifdef FORCE_AVX512
#include "IntelIntrinsics.h"
#endif

namespace ZSharp {

void Framebuffer::Initialize(size_t width, size_t height, size_t stride) {
  ZAssert(mPixelBuffer == nullptr);
  if (mPixelBuffer != nullptr) {
    Reset();
  }

  mWidth = width;
  mHeight = height;
  mStride = stride;
  mTotalSize = stride * height;
  mPixelBuffer = static_cast<uint8*>(PlatformAlignedMalloc(mTotalSize, 64));
#ifdef FORCE_AVX512
  mScratchBuffer = static_cast<uint8*>(PlatformAlignedMalloc(64, 64));
#endif
}

Framebuffer::Framebuffer() {
}

Framebuffer::~Framebuffer() {
  Reset();
}

void Framebuffer::SetPixel(const size_t x, const size_t y, const ZColor color) {
  if ((x >= 0 && x < mWidth) && (y >= 0 && y < mHeight)) {
    size_t offset = (x * sizeof(uint32)) + (y * mStride);
    *(reinterpret_cast<uint32*>(mPixelBuffer + offset)) = color.Color;
  }
}

void Framebuffer::SetRow(const size_t y, const size_t x1, const size_t x2, const ZColor color) {
  if ((y >= 0 && y < mHeight) &&
    (x1 >= 0 && x1 < mWidth) &&
    (x2 >= 0 && x2 < mWidth) &&
    (x1 < x2)) {
    size_t offset = (x1 * sizeof(uint32)) + (y * mStride);

    uint32* pixelPtr = reinterpret_cast<uint32*>(mPixelBuffer + offset);
    for (size_t i = 0; i < x2 - x1; ++i) {
      pixelPtr[i] = color.Color;
    }
  }
}

void Framebuffer::Clear(const ZColor color) {
  const size_t numPixels = (mWidth * mHeight);
#if FORCE_AVX512
  if ((numPixels % 64) > 0) {
    uint32* pixelPtr = reinterpret_cast<uint32*>(mPixelBuffer);
    for (size_t i = 0; i < numPixels; ++i) {
      pixelPtr[i] = color.Color;
    }
  }
  else {
    for (size_t i = 0; i < 16; ++i) {
      *(reinterpret_cast<uint32*>(mScratchBuffer) + i) = color.Color;
    }

    aligned_avx512memset(mPixelBuffer, mScratchBuffer, mTotalSize);
  }
#else
  if ((numPixels % sizeof(std::uintptr_t)) > 0) {
    uint32* pixelPtr = reinterpret_cast<uint32*>(mPixelBuffer);
    for (size_t i = 0; i < numPixels; ++i) {
      pixelPtr[i] = color.Color;
    }
  }
  else {
    const size_t cachedSize = mTotalSize / sizeof(std::uintptr_t);
    std::uintptr_t* pBuf = reinterpret_cast<std::uintptr_t*>(mPixelBuffer);
    const std::uintptr_t convColor = (static_cast<std::uintptr_t>(color.Color) << 32) | color.Color;

    for (size_t i = 0; i < cachedSize; i++) {
      pBuf[i] = convColor;
    }
  }
#endif
}

uint8* Framebuffer::GetBuffer() {
  return mPixelBuffer;
}

size_t Framebuffer::GetWidth() const {
  return mWidth;
}

size_t Framebuffer::GetHeight() const {
  return mHeight;
}

size_t Framebuffer::GetSize() const {
  return mTotalSize;
}

void Framebuffer::Resize() {
  if (mPixelBuffer != nullptr) {
    PlatformAlignedFree(mPixelBuffer);
  }

  mWidth = ZConfig::GetInstance().GetViewportWidth();
  mHeight = ZConfig::GetInstance().GetViewportHeight();
  mStride = mWidth * 4;
  mTotalSize = mStride * mHeight;
  mPixelBuffer = static_cast<uint8*>(PlatformAlignedMalloc(mTotalSize, 64));
}

void Framebuffer::Reset() {
  if (mPixelBuffer != nullptr) {
    PlatformAlignedFree(mPixelBuffer);
  }

#ifdef FORCE_AVX512
  if (mScratchBuffer != nullptr) {
    PlatformAlignedFree(mScratchBuffer);
  }
#endif
}

}
