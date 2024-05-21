#include "Texture.h"

#include "PlatformMemory.h"
#include "PlatformIntrinsics.h"

#include "ZAssert.h"

namespace ZSharp {
Texture::Texture() : mMipChain(1) {
}

Texture::Texture(uint8* data,
  size_t numChannels,
  size_t width,
  size_t height)
: mMipChain(1),
  mNumChannels(numChannels) {
  MipMap map;
  map.height = height;
  map.width = width;
  map.stride = width * 4;
  map.data = data;
  mMipChain[0] = map;
}

Texture::~Texture() {
  for (MipMap& map : mMipChain) {
    if (map.data != nullptr) {
      PlatformFree(map.data);
    }
  }
}

void Texture::Assign(uint8* data, size_t numChannels, size_t width, size_t height) {
  MipMap& map = mMipChain[0];
  if (map.data == nullptr) {
    map.data = data;
    mNumChannels = numChannels;
    map.width = width;
    map.stride = (width * numChannels);
    map.height = height;
  }
  else {
    ZAssert(false);
  }
}

bool Texture::IsAssigned() const {
  return mMipChain[0].data != nullptr;
}

size_t Texture::Width(size_t mipLevel) const {
  return mMipChain[mipLevel].width;
}

size_t Texture::Height(size_t mipLevel) const {
  return mMipChain[mipLevel].height;
}

size_t Texture::Channels() const {
  return mNumChannels;
}

uint8* Texture::Data(size_t mipLevel) const {
  return mMipChain[mipLevel].data;
}

void Texture::GenerateMips() {
  if (!IsAssigned()) {
    return;
  }

  size_t lastMip = 0;
  for (size_t width = mMipChain[0].width, height = mMipChain[0].height; width != 1 && height != 1; width >>= 1, height >>= 1, ++lastMip) {
    uint8* nextMipData = (uint8*)PlatformMalloc((width >> 1) * (height >> 1) * 4);
    uint8* lastMipData = mMipChain[lastMip].data;
    size_t nextMipStride = (width >> 1) * 4;
    size_t lastMipStride = width * 4;

    MipMap nextMip;
    nextMip.width = width >> 1;
    nextMip.stride = (width >> 1) * 4;
    nextMip.height = height >> 1;
    nextMip.data = nextMipData;

    for (size_t y = 0; y != (height >> 1); ++y) {
      for (size_t x = 0; x != (width >> 1); ++x) {
        uint8* topLeft = lastMipData + (y * 2 * lastMipStride) + ((x * 4) * 2);
        uint8* topRight = topLeft + 4;
        uint8* bottomLeft = lastMipData + (y * 2 * lastMipStride) + ((x * 4) * 2) + lastMipStride;
        uint8* bottomRight = bottomLeft + 4;

        uint8 B = (uint8)(((size_t)topLeft[0] + (size_t)topRight[0] + (size_t)bottomLeft[0] + (size_t)bottomRight[0]) / 4);
        uint8 G = (uint8)(((size_t)topLeft[1] + (size_t)topRight[1] + (size_t)bottomLeft[1] + (size_t)bottomRight[1]) / 4);
        uint8 R = (uint8)(((size_t)topLeft[2] + (size_t)topRight[2] + (size_t)bottomLeft[2] + (size_t)bottomRight[2]) / 4);
        uint8 A = (uint8)(((size_t)topLeft[3] + (size_t)topRight[3] + (size_t)bottomLeft[3] + (size_t)bottomRight[3]) / 4);
        
        nextMipData[(y * nextMipStride) + (x * 4)] = B;
        nextMipData[(y * nextMipStride) + (x * 4) + 1] = G;
        nextMipData[(y * nextMipStride) + (x * 4) + 2] = R;
        nextMipData[(y * nextMipStride) + (x * 4) + 3] = A;
      }
    }

    mMipChain.EmplaceBack(nextMip);
  }
}

uint8* InsertAlphaChannel(uint8* data, size_t width, size_t height) {
  uint8* alphaImage = (uint8*)PlatformMalloc(width * height * 4);
  Unaligned_BGRToBGRA(data, alphaImage, width * height * 3);
  PlatformFree(data);
  return alphaImage;
}

}
