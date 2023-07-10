#pragma once

#include "ZBaseTypes.h"

namespace ZSharp {

template<typename T>
void Swap(T& lhs, T& rhs) {
  const T temp(rhs);
  rhs = lhs;
  lhs = temp;
}

uint32 EndianSwap(uint32 value);

}
