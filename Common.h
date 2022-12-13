#pragma once

#include <cmath>

namespace ZSharp {

template<typename T>
void Swap(T& lhs, T& rhs) {
  const T temp(rhs);
  rhs = lhs;
  lhs = temp;
}

}
