#include "HashFunctions.h"

namespace ZSharp {

inline uint32 getblock32(const uint32* p, int32 i) {
  return p[i];
}

inline uint32 rotl32(uint32 x, int8 r) {
  return (x << r) | (x >> (32 - r));
}

inline uint32 fmix32(uint32 h) {
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;
  return h;
}

// Murmur3 32-bit hash algorithm.
// https://github.com/aappleby/smhasher/tree/master/src
uint32 MurmurHash3_32(const void* key, int32 length, uint32 seed) {
  const uint8* data = (const uint8*)key;
  const int nblocks = length / 4;

  uint32 h1 = seed;

  const uint32 c1 = 0xcc9e2d51;
  const uint32 c2 = 0x1b873593;

  //----------
  // body

  const uint32* blocks = (const uint32*)(data + nblocks * 4);

  for (int32 i = -nblocks; i; i++)
  {
    uint32 k1 = getblock32(blocks, i);

    k1 *= c1;
    k1 = rotl32(k1, 15);
    k1 *= c2;

    h1 ^= k1;
    h1 = rotl32(h1, 13);
    h1 = h1 * 5 + 0xe6546b64;
  }

  //----------
  // tail

  const uint8* tail = (const uint8*)(data + nblocks * 4);

  uint32 k1 = 0;

  switch (length & 3)
  {
    case 3: 
      k1 ^= tail[2] << 16;
#if __cplusplus >= 201703L
      [[fallthrough]];
#endif
    case 2: 
      k1 ^= tail[1] << 8;
#if __cplusplus >= 201703L
      [[fallthrough]];
#endif
    case 1: 
      k1 ^= tail[0];
      k1 *= c1; k1 = rotl32(k1, 15); k1 *= c2; h1 ^= k1;
  };

  //----------
  // finalization

  h1 ^= length;

  return fmix32(h1);
}

}
