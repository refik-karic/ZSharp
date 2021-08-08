#pragma once

#include <cstddef>

namespace ZSharp {

double DegreesToRadians(double degrees);

template <typename T>
T NewtonRaphsonSqrt(T val) {
  std::size_t iterations = 10;

  T x0 = {};
  x0 += 1;

  for (std::size_t i = 0; i < iterations; ++i) {
    x0 = x0 - (((x0 * x0) - val) / (2 * x0));
  }

  return x0;
}
}
