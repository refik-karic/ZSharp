#include "AABB.h"

#include "CommonMath.h"
#include "PlatformIntrinsics.h"

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

void AABB::Serialize(ISerializer& serializer) {
  serializer.Serialize(&mMin, sizeof(mMin));
  serializer.Serialize(&mMax, sizeof(mMax));
}

void AABB::Deserialize(IDeserializer& deserializer) {
  deserializer.Deserialize(&mMin, sizeof(mMin));
  deserializer.Deserialize(&mMax, sizeof(mMax));
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

  return Unaligned_AABB_Intersects((float*)&mMin, (float*)&mMax, (float*)&rhs.mMin, (float*)&rhs.mMax);
}

Vec3 AABB::Centroid() const {
  return (mMin + ((mMax - mMin) * 0.5f));
}

void AABB::Translate(const Vec3& translation) {
  mMin += translation;
  mMax += translation;
}

void AABB::ToPoints(Vec3 points[8]) const {
  const float* min = *mMin;
  const float* max = *mMax;

  points[0] = { min[0], min[1], max[2] };
  points[1] = { max[0], min[1], max[2] };
  points[2] = { max[0], max[1], max[2] };
  points[3] = { min[0], max[1], max[2] };

  points[4] = { min[0], min[1], min[2] };
  points[5] = { max[0], min[1], min[2] };
  points[6] = { max[0], max[1], min[2] };
  points[7] = { min[0], max[1], min[2] };
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
          outMin[i] += e;
          outMax[i] += f;
        }
        else {
          outMin[i] += f;
          outMax[i] += e;
        }
      }
    }
  */

  const float* inMin = (const float*)&inAABB.mMin;
  const float* inMax = (const float*)&inAABB.mMax;

  const float* matData = (const float*)&matrix;

  float outMin[4] = { matData[3], matData[7], matData[11], 0.f };
  float outMax[4] = { matData[3], matData[7], matData[11], 0.f };

  Unaligned_AABB_TransformAndRealign(inMin, inMax, outMin, outMax, matData);
  AABB outAABB(outMin, outMax);
  return outAABB;
}

}
