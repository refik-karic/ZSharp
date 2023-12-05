#pragma once

#include "ZBaseTypes.h"

#include "Vec3.h"
#include "Mat4x4.h"

namespace ZSharp {

class AABB final {
  public:

  AABB();

  AABB(const AABB& rhs);

  AABB(const float min[3], const float max[3]);

  void operator=(const AABB& rhs);

  void Grow(float vertex[3]);

  const Vec3& MinBounds() const;

  const Vec3& MaxBounds() const;

  bool Intersects(const AABB& rhs) const;

  Vec3 Centroid() const;

  void Translate(const Vec3& translation);

  void ToPoints(Vec3 points[8]) const;

  static AABB TransformAndRealign(const AABB& inAABB, const Mat4x4& matrix);

  private:
  Vec3 mMin;
  Vec3 mMax;
};

}
