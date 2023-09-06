#include "AABB.h"

#include "CommonMath.h"

#include <cmath>

namespace ZSharp {
AABB::AABB() 
  : mMin(0.f), mMax(0.f) {
}

AABB::AABB(const float min[3], const float max[3]) 
  : mMin(min[0], min[1], min[2]), mMax(max[0], max[1], max[2]) {
}

void AABB::Grow(float vertex[3]) {
  float* min = *mMin;
  float* max = *mMax;

  min[0] = Min(min[0], vertex[0]);
  min[1] = Min(min[1], vertex[1]);
  min[2] = Min(min[2], vertex[2]);

  max[0] = Max(max[0], vertex[0]);
  max[1] = Max(max[1], vertex[1]);
  max[2] = Max(max[2], vertex[2]);
}

const Vec3& AABB::MinBounds() const {
  return mMin;
}

const Vec3& AABB::MaxBounds() const {
  return mMax;
}

}
