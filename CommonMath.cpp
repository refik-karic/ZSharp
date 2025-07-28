#include "CommonMath.h"

#include "ScopedTimer.h"
#include "PlatformIntrinsics.h"

namespace ZSharp {
size_t RoundUpNearestMultiple(size_t val, size_t multiple) {
  if (val == 0 || multiple == 0) {
    ZAssert(false);
    return 0;
  }

  return ((val + multiple - 1) / multiple) * multiple;
}

size_t RoundDownNearestMultiple(size_t val, size_t multiple) {
  if (val == 0 || multiple == 0) {
    ZAssert(false);
    return 0;
  }

  return (val / multiple) * multiple;
}

uint32 RoundToNearestPowTwo(uint32 val) {
  --val;
  val |= val >> 1;
  val |= val >> 2;
  val |= val >> 4;
  val |= val >> 8;
  val |= val >> 16;
  ++val;
  return val;
}

size_t RoundToNearestPowTwo(size_t val) {
  --val;
  val |= val >> 1;
  val |= val >> 2;
  val |= val >> 4;
  val |= val >> 8;
  val |= val >> 16;
  val |= val >> 32;
  ++val;
  return val;
}

float DegreesToRadians(float degrees) {
  return degrees * PI_OVER_180;
}

float ParametricSolveForT(const float step, const float p0, const float p1) {
  const float numerator = step - p0;
  const float denominator = p1 - p0;
  return numerator / denominator;
}

float NewtonRaphsonSqrt(float val) {
    const size_t iterations = 10;

    float x0 = 0.f;
    x0 += 1.f;

    for (size_t i = 0; i < iterations; ++i) {
        x0 = x0 - (((x0 * x0) - val) / (2.f * x0));
    }

    return x0;
}

bool IsNegativeFloat(const float a) {
    const uint8* floatBits = reinterpret_cast<const uint8*>(&a);
    return (((floatBits[3]) & 0x01) > 0);
}

bool FloatEqual(float a, float b, float epsilon) {
  return (fabs(a - b) <= epsilon);
}

bool FloatLessThan(float a, float b, float epsilon) {
  return (!FloatEqual(a, b, epsilon)) ? a < b : false;
}

bool FloatLessThanEqual(float a, float b, float epsilon) {
  return (FloatEqual(a, b, epsilon)) ? true : a < b;
}

bool FloatGreaterThan(float a, float b, float epsilon) {
  return (!FloatEqual(a, b, epsilon)) ? a > b : false;
}

bool FloatGreaterThanEqual(float a, float b, float epsilon) {
  return (FloatEqual(a, b, epsilon)) ? true : a > b;
}

AABB ComputeBoundingBox(size_t stride, const float* verts, const size_t numVerts) {
  NamedScopedTimer(ComputeAABB);

  float min[4] = { INFINITY, INFINITY, INFINITY, INFINITY };
  float max[4] = { -INFINITY, -INFINITY, -INFINITY, -INFINITY };

  CalculateAABBImpl(verts, numVerts, stride, min, max);

  AABB aabb(min, max);
  return aabb;
}

}
