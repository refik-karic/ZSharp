#include "Texture.h"

#include "PlatformMemory.h"
#include "PlatformIntrinsics.h"

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
    mData = nullptr;
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

size_t Texture::Width() const {
  return mWidth;
}

size_t Texture::Height() const {
  return mHeight;
}

size_t Texture::Channels() const {
  return mNumChannels;
}

uint8* Texture::Data() const {
  return mData;
}

uint8* InsertAlphaChannel(uint8* data, size_t width, size_t height) {
  uint8* alphaImage = (uint8*)PlatformMalloc(width * height * 4);
  Unaligned_BGRToBGRA(data, alphaImage, width * height * 3);
  PlatformFree(data);
  return alphaImage;
}

}
