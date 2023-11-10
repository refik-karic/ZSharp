#include "AABB.h"

#include "CommonMath.h"

#include <cmath>

namespace ZSharp {
AABB::AABB() 
  : mMin(0.f), mMax(0.f) {
}

AABB::AABB(const AABB& rhs) : mMin(rhs.mMin), mMax(rhs.mMax) {
}

AABB::AABB(const float min[3], const float max[3]) 
  : mMin(min[0], min[1], min[2]), mMax(max[0], max[1], max[2]) {
}

void AABB::operator=(const AABB& rhs) {
  if (this == &rhs) {
    return;
  }

  mMin = rhs.mMin;
  mMax = rhs.mMax;
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

bool AABB::Intersects(const AABB& rhs) const {
  /*
    Test each axis to see if there's an overlap.
    The AABBs only overlap if all axes overlap at a point.
  */

  if (mMax[0] < rhs.mMin[0] || mMin[0] > rhs.mMax[0]) {
    return false;
  }
  else if (mMax[1] < rhs.mMin[1] || mMin[1] > rhs.mMax[1]) {
    return false;
  }
  else if (mMax[2] < rhs.mMin[2] || mMin[2] > rhs.mMax[2]) {
    return false;
  }
  else {
    return true;
  }
}

Vec3 AABB::Centroid() const {
  return (mMin + ((mMax - mMin) * 0.5f));
}

AABB AABB::TransformAndRealign(const AABB& inAABB, const Mat4x4& matrix) {
  /*
    This applies the transform to the input AABB and realigns it to the axes.
    Note that it will not fit as tightly as the original but it's still significantly quicker to perform this over recalculating the entire AABB.

    The following is an unrolled version of:

    float outMin[3] = {matrix[0][3], matrix[1][3], matrix[2][3]};
    float outMax[3] = {matrix[0][3], matrix[1][3], matrix[2][3]};

    for(size_t i = 0; i < 3; ++i) {
      for(size_t j = 0; j < 3; ++j) {
        float e = matrix[i][j] * inMin[j];
        float f = matrix[i][j] * inMax[j];

        if(e < f) {
          outMin[j] += e;
          outMax[j] += f;
        }
        else {
          outMin[j] += f;
          outMax[j] += e;
        }
      }
    }
  */

  const float* inMin = *inAABB.MinBounds();
  const float* inMax = *inAABB.MaxBounds();

  float outMin[3] = { matrix[0][3], matrix[1][3], matrix[2][3] };
  float outMax[3] = { matrix[0][3], matrix[1][3], matrix[2][3] };

  float e00 = matrix[0][0] * inMin[0];
  float f00 = matrix[0][0] * inMax[0];

  if (e00 < f00) {
    outMin[0] += e00;
    outMax[0] += f00;
  }
  else {
    outMin[0] += f00;
    outMax[0] += e00;
  }

  float e01 = matrix[0][1] * inMin[1];
  float f01 = matrix[0][1] * inMax[1];

  if (e01 < f01) {
    outMin[0] += e01;
    outMax[0] += f01;
  }
  else {
    outMin[0] += f01;
    outMax[0] += e01;
  }

  float e02 = matrix[0][2] * inMin[2];
  float f02 = matrix[0][2] * inMax[2];

  if (e02 < f02) {
    outMin[0] += e02;
    outMax[0] += f02;
  }
  else {
    outMin[0] += f02;
    outMax[0] += e02;
  }

  float e10 = matrix[1][0] * inMin[0];
  float f10 = matrix[1][0] * inMax[0];

  if (e10 < f10) {
    outMin[1] += e10;
    outMax[1] += f10;
  }
  else {
    outMin[1] += f10;
    outMax[1] += e10;
  }

  float e11 = matrix[1][1] * inMin[1];
  float f11 = matrix[1][1] * inMax[1];

  if (e11 < f11) {
    outMin[1] += e11;
    outMax[1] += f11;
  }
  else {
    outMin[1] += f11;
    outMax[1] += e11;
  }

  float e12 = matrix[1][2] * inMin[2];
  float f12 = matrix[1][2] * inMax[2];

  if (e12 < f12) {
    outMin[1] += e12;
    outMax[1] += f12;
  }
  else {
    outMin[1] += f12;
    outMax[1] += e12;
  }

  float e20 = matrix[2][0] * inMin[0];
  float f20 = matrix[2][0] * inMax[0];

  if (e20 < f20) {
    outMin[2] += e20;
    outMax[2] += f20;
  }
  else {
    outMin[2] += f20;
    outMax[2] += e20;
  }

  float e21 = matrix[2][1] * inMin[1];
  float f21 = matrix[2][1] * inMax[1];

  if (e21 < f21) {
    outMin[2] += e21;
    outMax[2] += f21;
  }
  else {
    outMin[2] += f21;
    outMax[2] += e21;
  }

  float e22 = matrix[2][2] * inMin[2];
  float f22 = matrix[2][2] * inMax[2];

  if (e22 < f22) {
    outMin[2] += e22;
    outMax[2] += f22;
  }
  else {
    outMin[2] += f22;
    outMax[2] += e22;
  }

  AABB outAABB(outMin, outMax);
  return outAABB;
}

}
