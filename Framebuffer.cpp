#include "Framebuffer.h"

#include <cstdlib>

#include "Common.h"
#include "ZConfig.h"

#ifdef FORCE_AVX512
#include "IntelIntrinsics.h"
#endif

namespace ZSharp {
Framebuffer::Framebuffer(size_t width, size_t height, size_t stride) : 
  mWidth(width),
  mHeight(height),
  mStride(stride)
{
  mTotalSize = stride * height;
  mPixelBuffer = static_cast<uint8_t*>(_aligned_malloc(mTotalSize, 64));
#ifdef FORCE_AVX512
  mScratchBuffer = static_cast<uint8_t*>(_aligned_malloc(64, 64));
#endif
}

Framebuffer::~Framebuffer(){
  if(mPixelBuffer != nullptr){
    _aligned_free(mPixelBuffer);
  }

#ifdef FORCE_AVX512
  if (mScratchBuffer != nullptr) {
    _aligned_free(mScratchBuffer);
  }
#endif
}

void Framebuffer::SetPixel(const size_t x, const size_t y, const ZColor color) {
  if ((x >= 0 && x < mWidth) && (y >= 0 && y < mHeight)) {
    size_t offset = (x * sizeof(uint32_t)) + (y * mStride);
    *(reinterpret_cast<uint32_t*>(mPixelBuffer + offset)) = color.Color;
  }
}

void Framebuffer::SetRow(const size_t y, const size_t x1, const size_t x2, const ZColor color) {
  if ((y >= 0 && y < mHeight) &&
    (x1 >= 0 && x1 < mWidth) &&
    (x2 >= 0 && x2 < mWidth) &&
    (x1 < x2)) {
    size_t offset = (x1 * sizeof(uint32_t)) + (y * mStride);
    ZSharp::MemsetAny(reinterpret_cast<uint32_t*>(mPixelBuffer + offset),
      color.Color,
      x2 - x1);
  }
}

void Framebuffer::Clear(const ZColor color) {
  const size_t numPixels = (mWidth * mHeight);
#if FORCE_AVX512
  if ((numPixels % 64) > 0) {
    ZSharp::MemsetAny(reinterpret_cast<uint32_t*>(mPixelBuffer),
      color.Color,
      numPixels);
  }
  else {
    for (size_t i = 0; i < 16; ++i) {
      *(reinterpret_cast<uint32_t*>(mScratchBuffer) + i) = color.Color;
    }

    aligned_avx512memset(mPixelBuffer, mScratchBuffer, mTotalSize);
  }
#else
  if ((numPixels % sizeof(std::uintptr_t)) > 0) {
    ZSharp::MemsetAny(reinterpret_cast<uint32_t*>(mPixelBuffer),
      color.Color,
      numPixels);
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

uint8_t* Framebuffer::GetBuffer() {
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
    _aligned_free(mPixelBuffer);
  }

  mWidth = ZConfig::GetInstance().GetViewportWidth();
  mHeight = ZConfig::GetInstance().GetViewportHeight();
  mStride = mWidth * 4;
  mTotalSize = mStride * mHeight;
  mPixelBuffer = static_cast<uint8_t*>(_aligned_malloc(mTotalSize, 64));
}

}
