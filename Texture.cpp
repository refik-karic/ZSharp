#include "Texture.h"

#include "PlatformMemory.h"

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

  for (size_t h = 0; h < height; ++h) {
    uint8* inPtr = data + (h * width * 3);
    uint8* outPtr = alphaImage + (h * width * 4);
    for (size_t w = 0; w < width; ++w, inPtr += 3, outPtr += 4) {
      outPtr[0] = inPtr[0];
      outPtr[1] = inPtr[1];
      outPtr[2] = inPtr[2];
      outPtr[3] = 0xFF;
    }
  }

  PlatformFree(data);
  return alphaImage;
}

}
