#include "Texture.h"

#include "PlatformMemory.h"
#include "PlatformIntrinsics.h"

#include "ZAssert.h"
#include "ScopedTimer.h"

namespace ZSharp {
Texture::Texture() : mMipChain(1) {
}

Texture::Texture(uint8* data,
  size_t numChannels,
  size_t width,
  size_t height)
: mMipChain(1),
  mNumChannels(numChannels) {
  MipMap& map = mMipChain[0];
  map.height = height;
  map.width = width;
  map.stride = width * 4;
  map.data = data;
}

Texture::~Texture() {
  if (mMipData != nullptr) {
    PlatformAlignedFree(mMipData);
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

  NamedScopedTimer(GenerateMips);

  size_t lastMip = 0;
  size_t allocationSize = 0;
  for (size_t width = mMipChain[0].width, height = mMipChain[0].height; width != 1 && height != 1; width >>= 1, height >>= 1, ++lastMip) {
    allocationSize += (width >> 1) * (height >> 1) * 4;
  }

  allocationSize = RoundUpNearestMultiple(allocationSize, PlatformAlignmentGranularity());
  lastMip = 0;
  mMipData = (uint8*)PlatformAlignedMalloc(allocationSize, PlatformAlignmentGranularity());
  uint8* allocationOffset = mMipData;
  for (size_t width = mMipChain[0].width, height = mMipChain[0].height; width != 1 && height != 1; width >>= 1, height >>= 1, ++lastMip) {
    size_t mipWidth = width >> 1;
    size_t mipHeight = height >> 1;
    
    uint8* nextMipData = allocationOffset;
    uint8* lastMipData = mMipChain[lastMip].data;
    size_t nextMipStride = mipWidth * 4;
    allocationOffset += mipWidth * mipHeight * 4;

    MipMap nextMip;
    nextMip.width = mipWidth;
    nextMip.stride = nextMipStride;
    nextMip.height = mipHeight;
    nextMip.data = nextMipData;

    GenerateMipLevelImpl(nextMipData, mipWidth, mipHeight, lastMipData, width, height);

    mMipChain.EmplaceBack(nextMip);
  }
}

size_t Texture::NumMips() const {
  return mMipChain.Size();
}

uint8* InsertAlphaChannel(uint8* data, size_t width, size_t height) {
  uint8* alphaImage = (uint8*)PlatformMalloc(width * height * 4);
  Unaligned_BGRToBGRA(data, alphaImage, width * height * 3);
  PlatformFree(data);
  return alphaImage;
}

}
