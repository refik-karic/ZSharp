#pragma once

#include <cstddef>

namespace ZSharp {

float DegreesToRadians(float degrees);

float NewtonRaphsonSqrt(float val);

bool FloatEqual(float a, float b, float epsilon);

bool FloatLessThan(float a, float b, float epsilon);

bool FloatLessThanEqual(float a, float b, float epsilon);

bool FloatGreaterThan(float a, float b, float epsilon);

bool FloatGreaterThanEqual(float a, float b, float epsilon);

}
