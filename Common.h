#pragma once

#include "ZBaseTypes.h"
#include "MoveHelpers.h"

namespace ZSharp {

template<typename T>
void Swap(T& lhs, T& rhs) {
  T temp(Move(rhs));
  rhs = Move(lhs);
  lhs = Move(temp);
}

uint16 EndianSwap(uint16 value);

uint32 EndianSwap(uint32 value);

}
