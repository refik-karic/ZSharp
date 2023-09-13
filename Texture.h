#pragma once

#include "ZBaseTypes.h"
#include "ZColor.h"
#include "CommonMath.h"
#include "PlatformDefines.h"

namespace ZSharp {

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

  FORCE_INLINE ZColor Sample(float u, float v) const {
    Clamp(u, 0.f, 1.f);
    Clamp(v, 0.f, 1.f);

    const size_t x = static_cast<size_t>(u * (mWidth - 1));
    const size_t y = static_cast<size_t>(v * (mHeight - 1));
    const size_t pixel = (y * mStride) + (x * mNumChannels);

    const uint8 B = mData[pixel];
    const uint8 G = mData[pixel + 1];
    const uint8 R = mData[pixel + 2];

    ZColor color(R, G, B);
    return color;
  }

  private:
  size_t mNumChannels = 0;
  size_t mWidth = 0;
  size_t mStride = 0;
  size_t mHeight = 0;
  uint8* mData = nullptr;
};

}
