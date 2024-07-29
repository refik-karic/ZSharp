#pragma once

#include "ZBaseTypes.h"
#include "ZColor.h"
#include "CommonMath.h"
#include "PlatformDefines.h"
#include "Array.h"

namespace ZSharp {

uint8* InsertAlphaChannel(uint8* data, size_t width, size_t height);

/*
A 2D texture that owns its memory. The idea is to load some kind of standardized image/material format into an agnostic class.
The renderer can then sample from this texture using U,V's.
U = Horiztonal [0..1]
V = Vertical [0..1]
Origin is top left of the texture at [0,0]
We don't handle any fancy channels right now, assuming native RGB display layout.
Clamping is enforced and not configurable. This is set to [0..1].
We do not do any filtering at the moment.
*/
class Texture final {
  public:

  Texture();

  Texture(uint8* data,
    size_t numChannels,
    size_t width,
    size_t height);

  Texture(const Texture& rhs) = delete;

  ~Texture();

  void Assign(uint8* data,
    size_t numChannels,
    size_t width,
    size_t height);

  bool IsAssigned() const;

  FORCE_INLINE uint32 Sample(float u, float v, size_t mipLevel) const {
    /*
      Note that we don't do any kind of check/clamp to make sure U,V are within a valid range.
      This can easily be checked in the raster loop by making sure all attributes are in the proper range.
    */
    const MipMap& map = mMipChain[mipLevel];

    const size_t x = static_cast<size_t>(u * (map.width - 1));
    const size_t y = static_cast<size_t>(v * (map.height - 1));
    const size_t pixel = (y * map.stride) + (x * mNumChannels);

    // We're assuming the texture channel layout matches the display here.
    // This doesn't make a lot of sense for non-albedo textures but we can take care of that later.
    return *((uint32*)(mMipChain[mipLevel].data + pixel));
  }

  size_t Width(size_t mipLevel) const;

  size_t Height(size_t mipLevel) const;

  size_t Channels() const;

  uint8* Data(size_t mipLevel) const;

  void GenerateMips();

  size_t NumMips() const;

  private:
  size_t mNumChannels = 0;
  struct MipMap {
    size_t width = 0;
    size_t height = 0;
    size_t stride = 0;
    uint8* data = nullptr;
  };
  Array<MipMap> mMipChain;
  uint8* mMipData = nullptr;
};

}
