#pragma once

#include "ZBaseTypes.h"

namespace ZSharp {

class Texture final {
  public:

  Texture(uint8* data,
    size_t bitsPerPixel, 
    size_t numChannels,
    size_t totalSize,
    size_t width,
    size_t height);

  Texture(const Texture& rhs) = delete;

  ~Texture();

  private:
  size_t mBitsPerPixel = 0;
  size_t mNumChannels = 0;
  size_t mTotalSize = 0;
  size_t mWidth = 0;
  size_t mHeight = 0;
  uint8* mPixelData = nullptr;
};

}
