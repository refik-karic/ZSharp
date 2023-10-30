#pragma once

#include "AABB.h"
#include "ZBaseTypes.h"
#include "Vec3.h"
#include "WorldObject.h"

namespace ZSharp {

class PhysicsObject : public WorldObject {
  public:

  AABB& BoundingBox();

  const AABB& BoundingBox() const;

  Vec3& Velocity();

  const Vec3& Velocity() const;

  private:
  Vec3 mVelocity;
  AABB mBoundingBox;
};

}
