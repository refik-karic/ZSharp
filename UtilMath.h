#pragma once

#include <cstddef>

namespace ZSharp {

float DegreesToRadians(float degrees);

float Lerp(float x1, float x2, float point);

float NewtonRaphsonSqrt(float val);

bool IsNegativeFloat(const float a);

bool FloatEqual(float a, float b, float epsilon = 1.e-5f);

bool FloatLessThan(float a, float b, float epsilon = 1.e-5f);

bool FloatLessThanEqual(float a, float b, float epsilon = 1.e-5f);

bool FloatGreaterThan(float a, float b, float epsilon = 1.e-5f);

bool FloatGreaterThanEqual(float a, float b, float epsilon = 1.e-5f);

}
