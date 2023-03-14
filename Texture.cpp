#include "Texture.h"

#include "PlatformMemory.h"
#include "CommonMath.h"

namespace ZSharp {

Texture::Texture(uint8* data,
  size_t bytesPerPixel,
  size_t numChannels,
  size_t width,
  size_t height)
: mData(data),
  mBytesPerPixel(bytesPerPixel),
  mNumChannels(numChannels),
  mWidth(width),
  mStride(width * bytesPerPixel),
  mHeight(height) {
  
}

Texture::~Texture() {
  if (mData != nullptr) {
    PlatformFree(mData);
  }
}

ZColor Texture::Sample(float u, float v) const {
  Clamp(u, 0.f, 1.f);
  Clamp(v, 0.f, 1.f);

  size_t x = u * mWidth;
  x *= mBytesPerPixel;
  size_t y = v * mHeight;
  size_t yStride = (y * mStride);
  size_t pixel = yStride + x;

  uint8 B = mData[pixel];
  uint8 G = mData[pixel + 1];
  uint8 R = mData[pixel + 2];

  ZColor color(R, G, B);
  return color;
}

}
