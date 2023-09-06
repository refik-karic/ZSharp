#pragma once

#include "ZBaseTypes.h"

#include "Vec3.h"

namespace ZSharp {

class AABB final {
  public:

  AABB();

  AABB(const float min[3], const float max[3]);

  void Grow(float vertex[3]);

  const Vec3& MinBounds() const;

  const Vec3& MaxBounds() const;

  private:
  Vec3 mMin;
  Vec3 mMax;
};

}
