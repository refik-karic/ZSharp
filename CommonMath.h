#pragma once

#include "ZBaseTypes.h"

namespace ZSharp {

size_t RoundUpNearestMultiple(size_t val, size_t multiple);

float DegreesToRadians(float degrees);

float Lerp(float x1, float x2, float point);

float ParametricSolveForT(const float step, const float p0, const float p1);

float NewtonRaphsonSqrt(float val);

bool IsNegativeFloat(const float a);

bool FloatEqual(float a, float b, float epsilon = 1.e-5f);

bool FloatLessThan(float a, float b, float epsilon = 1.e-5f);

bool FloatLessThanEqual(float a, float b, float epsilon = 1.e-5f);

bool FloatGreaterThan(float a, float b, float epsilon = 1.e-5f);

bool FloatGreaterThanEqual(float a, float b, float epsilon = 1.e-5f);

}
