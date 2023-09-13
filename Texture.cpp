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

}
