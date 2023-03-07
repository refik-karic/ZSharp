#include "CommonMath.h"

#include <cmath>

#include "Constants.h"
#include "ZAssert.h"

namespace ZSharp {
size_t RoundUpNearestMultiple(size_t val, size_t multiple) {
  if (val == 0 || multiple == 0) {
    ZAssert(false);
    return 0;
  }

  if ((val % multiple) > 0) {
    return (static_cast<size_t>(ceil((double)val / (double)multiple)) * multiple);
  }
  else {
    return val;
  }
}

float DegreesToRadians(float degrees) {
  return degrees * PI_OVER_180;
}

float Lerp(float x1, float x2, float point) {
  return fmaf(point, x2, (1.f - point) * x1);
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

}
