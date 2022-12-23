#pragma once

#include "ZBaseTypes.h"

namespace ZSharp {

// Fast and lightweight one dimensional noise based RNG.
// Takes an optional seed and is able to skip around using a position.

class NoiseRand final {
  public:

  NoiseRand(uint32 seed = 0);

  uint32 Rand();

  size_t& Position();

  private:
  size_t mPosition;
  uint32 mSeed;
};

}
