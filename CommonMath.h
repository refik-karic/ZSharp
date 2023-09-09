#pragma once

#include "ZAssert.h"
#include "ZBaseTypes.h"
#include "Constants.h"
#include "PlatformDefines.h"

#include <cmath>

namespace ZSharp {

template<typename T>
void Clamp(T& value, const T& min, const T& max) {
  ZAssert(min <= max);
  
  if (value < min) {
    value = min;
  }
  else if (value > max) {
    value = max;
  }
}

template<typename T>
T& Min(T& a, T& b) {
  return (a < b) ? a : b;
}

template<typename T>
T& Max(T& a, T& b) {
  return (a > b) ? a : b;
}

size_t RoundUpNearestMultiple(size_t val, size_t multiple);

float DegreesToRadians(float degrees);

FORCE_INLINE float Lerp(float x1, float x2, float point) {
  return fmaf(point, x2, (1.f - point) * x1);
}

float ParametricSolveForT(const float step, const float p0, const float p1);

float NewtonRaphsonSqrt(float val);

bool IsNegativeFloat(const float a);

bool FloatEqual(float a, float b, float epsilon = DEFAULT_EPSILON);

bool FloatLessThan(float a, float b, float epsilon = DEFAULT_EPSILON);

bool FloatLessThanEqual(float a, float b, float epsilon = DEFAULT_EPSILON);

bool FloatGreaterThan(float a, float b, float epsilon = DEFAULT_EPSILON);

bool FloatGreaterThanEqual(float a, float b, float epsilon = DEFAULT_EPSILON);

}
