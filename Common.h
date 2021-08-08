#pragma once

#include <cstddef>

namespace ZSharp {

template<typename T>
void MemsetAny(T* dest, const T& value, std::size_t length) {
  for(std::size_t i = 0; i < length; ++i) {
    dest[i] = value;
  }
}

}
