#pragma once

#include "ZAssert.h"
#include "ZBaseTypes.h"
#include "Constants.h"
#include "PlatformDefines.h"

#include <cmath>

namespace ZSharp {

bool IsNegativeFloat(const float a);

bool FloatEqual(float a, float b, float epsilon = DEFAULT_EPSILON);

bool FloatLessThan(float a, float b, float epsilon = DEFAULT_EPSILON);

bool FloatLessThanEqual(float a, float b, float epsilon = DEFAULT_EPSILON);

bool FloatGreaterThan(float a, float b, float epsilon = DEFAULT_EPSILON);

bool FloatGreaterThanEqual(float a, float b, float epsilon = DEFAULT_EPSILON);

FORCE_INLINE void Sub3(const float lhs[3], const float rhs[3], float out[3]) {
  out[0] = lhs[0] - rhs[0];
  out[1] = lhs[1] - rhs[1];
  out[2] = lhs[2] - rhs[2];
}

FORCE_INLINE float Dot3(const float lhs[3], const float rhs[3]) {
  return (lhs[0] * rhs[0]) +
    (lhs[1] * rhs[1]) +
    (lhs[2] * rhs[2]);
}

template<typename T>
FORCE_INLINE const T& Clamp(const T& value, const T& min, const T& max) {
  ZAssert(min <= max);
  
  if (value < min) {
    return min;
  }
  else if (value > max) {
    return max;
  }
  else {
    return value;
  }
}

template<typename T>
FORCE_INLINE T& Min(T& a, T& b) {
  return (a < b) ? a : b;
}

template<typename T>
FORCE_INLINE T& Max(T& a, T& b) {
  return (a > b) ? a : b;
}

size_t RoundUpNearestMultiple(size_t val, size_t multiple);

float DegreesToRadians(float degrees);

FORCE_INLINE float Lerp(float x1, float x2, float point) {
  return fmaf(point, x2, (1.f - point) * x1);
}

FORCE_INLINE float BarycentricArea2D(const float v1[2], const float v2[2], const float v3[2]) {
  /*
  This is an optimized 2d determinant calculation.
  
  The way this works is you pass the 3 triangle verts to get the area.
  Then you can pass the point to test once for each side and divide by the initial area to get u,v,w.

  This doesn't hold true in 3d which is why we have different ways of calculating these.
  */
  return ((v3[0] - v1[0]) * (v2[1] - v1[1])) - ((v3[1] - v1[1]) * (v2[0] - v1[0]));
}

FORCE_INLINE void BarycentricArea3D(const float v1[3], const float v2[3], const float v3[3], const float p[3], float uvw[3]) {
  float vb0[3];
  float vb1[3];
  float vb2[3];

  Sub3(v2, v1, vb0);
  Sub3(v3, v1, vb1);
  Sub3(p, v1, vb2);

  float d00 = Dot3(vb0, vb0);
  float d01 = Dot3(vb0, vb1);
  float d11 = Dot3(vb1, vb1);
  float d20 = Dot3(vb2, vb0);
  float d21 = Dot3(vb2, vb1);

  float denominator = (d00 * d11) - (d01 * d01);

  float v = ((d11 * d20) - (d01 * d21)) / denominator;
  float w = ((d00 * d21) - (d01 * d20)) / denominator;
  float u = 1.f - v - w;

  uvw[0] = u;
  uvw[1] = v;
  uvw[2] = w;
}

float ParametricSolveForT(const float step, const float p0, const float p1);

float NewtonRaphsonSqrt(float val);

}
