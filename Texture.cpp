#include "Texture.h"

#include "PlatformMemory.h"
#include "CommonMath.h"

#include "ZAssert.h"

namespace ZSharp {
Texture::Texture() {
}

Texture::Texture(uint8* data,
  size_t numChannels,
  size_t width,
  size_t height)
: mData(data),
  mNumChannels(numChannels),
  mWidth(width),
  mStride(width * numChannels),
  mHeight(height) {
  
}

Texture::~Texture() {
  if (mData != nullptr) {
    PlatformFree(mData);
  }
}

void Texture::Assign(uint8* data, size_t numChannels, size_t width, size_t height) {
  if (mData == nullptr) {
    mData = data;
    mNumChannels = numChannels;
    mWidth = width;
    mStride = (width * numChannels);
    mHeight = height;
  }
  else {
    ZAssert(false);
  }
}

bool Texture::IsAssigned() const {
  return mData != nullptr;
}

ZColor Texture::Sample(float u, float v) const {
  Clamp(u, 0.f, 1.f);
  Clamp(v, 0.f, 1.f);

  size_t x = static_cast<size_t>(u * (mWidth - 1));
  x *= mNumChannels;
  size_t y = static_cast<size_t>(v * (mHeight - 1));
  size_t yStride = (y * mStride);
  size_t pixel = yStride + x;

  uint8 B = mData[pixel];
  uint8 G = mData[pixel + 1];
  uint8 R = mData[pixel + 2];

  ZColor color(R, G, B);
  return color;
}

}
