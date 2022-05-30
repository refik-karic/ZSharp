#include "UtilMath.h"

#include <cmath>

#include "Constants.h"

namespace ZSharp {

float DegreesToRadians(float degrees) {
  return degrees * PI_OVER_180;
}

float Lerp(float x1, float x2, float point) {
  return ((1 - point) * x1) + (point * x2);
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
