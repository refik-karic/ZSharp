#pragma once

#include <cstddef>

namespace ZSharp {

template<typename T>
void MemsetAny(T* const dest, const T& value, const std::size_t length) {
  for(std::size_t i = 0; i < length; ++i) {
    dest[i] = value;
  }
}

}
