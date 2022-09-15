#pragma once

#include "ZBaseTypes.h"
#include "ZString.h"

#include <cstring>

namespace ZSharp {

uint32 MurmurHash3_32(const void* key, int32 length, uint32 seed);

template<typename T>
struct Hash {
  uint32 operator()(const T& key) const {
    return MurmurHash3_32(&key, sizeof(key), 0);
  }
};

template<>
struct Hash<char*> {
  uint32 operator()(const char*& key) const {
    return MurmurHash3_32(key, (int32)strlen(key), 0);
  }
};

template<>
struct Hash<String> {
  uint32 operator()(const String& key) const {
    return MurmurHash3_32(key.Str(), (int32)key.Length(), 0);
  }
};

}
