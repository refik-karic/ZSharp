#include "UtilMath.h"

#include <cmath>

#include "Constants.h"

namespace ZSharp {

float DegreesToRadians(float degrees) {
  return degrees * Constants::PI_OVER_180;
}

float NewtonRaphsonSqrt(float val) {
    const std::size_t iterations = 10;

    float x0 = 0.f;
    x0 += 1.f;

    for (std::size_t i = 0; i < iterations; ++i) {
        x0 = x0 - (((x0 * x0) - val) / (2.f * x0));
    }

    return x0;
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
