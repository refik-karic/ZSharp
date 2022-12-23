#include "Random.h"

#include "HashFunctions.h"

namespace ZSharp {

NoiseRand::NoiseRand(uint32 seed)
  : mPosition(0), mSeed(seed) {

}

uint32 NoiseRand::Rand() {
  uint32 result = MurmurHash3_32(&mPosition, sizeof(mPosition), mSeed);
  ++mPosition;
  return result;
}

size_t& NoiseRand::Position() {
  return mPosition;
}

}
