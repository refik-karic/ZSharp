#include "Common.h"
#include "Framebuffer.h"

#include <cstdlib>

#ifdef __AVX512F__
#include "IntelIntrinsics.h"
#endif

namespace ZSharp {
Framebuffer::Framebuffer(std::size_t width, 
  std::size_t height,
  std::size_t stride) :
  mWidth(width),
  mHeight(height),
  mStride(stride)
{
  mTotalSize = stride * height;
  mPixelBuffer = static_cast<std::uint8_t*>(_aligned_malloc(mTotalSize, 64));
  mScratchBuffer = static_cast<std::uint8_t*>(_aligned_malloc(64, 64));
}

Framebuffer::~Framebuffer(){
  if(mPixelBuffer != nullptr){
    _aligned_free(mPixelBuffer);
  }

  if (mScratchBuffer != nullptr) {
    _aligned_free(mScratchBuffer);
  }
}

void Framebuffer::SetPixel(const std::size_t x, const std::size_t y, const ZColor color) {
  if ((x >= 0 && x < mWidth) && (y >= 0 && y < mHeight)) {
    std::size_t offset = (x * sizeof(std::uint32_t)) + (y * mStride);
    *(reinterpret_cast<std::uint32_t*>(mPixelBuffer + offset)) = color.Color;
  }
}

void Framebuffer::SetRow(const std::size_t y, const std::size_t x1, const std::size_t x2, const ZColor color) {
  if ((y >= 0 && y < mHeight) &&
    (x1 >= 0 && x1 < mWidth) &&
    (x2 >= 0 && x2 < mWidth) &&
    (x1 < x2)) {
    std::size_t offset = (x1 * sizeof(std::uint32_t)) + (y * mStride);
    ZSharp::MemsetAny(reinterpret_cast<std::uint32_t*>(mPixelBuffer + offset),
      color.Color,
      x2 - x1);
  }
}

void Framebuffer::Clear(const ZColor color) {
#if __AVX512F__
  for (std::size_t i = 0; i < 16; ++i) {
    *(reinterpret_cast<std::uint32_t*>(mScratchBuffer) + i) = color.Color;
  }

  avx512memsetaligned(mPixelBuffer, mScratchBuffer, mTotalSize);
#else
  const std::size_t cachedSize = mTotalSize / sizeof(std::uintptr_t);
  std::uintptr_t* pBuf = reinterpret_cast<std::uintptr_t*>(mPixelBuffer);
  const std::uintptr_t convColor = (static_cast<std::uintptr_t>(color.Color) << 32) | color.Color;

  for (std::size_t i = 0; i < cachedSize; i++) {
    pBuf[i] = convColor;
  }
#endif
}

std::uint8_t* Framebuffer::GetBuffer() {
  return mPixelBuffer;
}

std::size_t Framebuffer::GetSize() const {
  return mTotalSize;
}

}
