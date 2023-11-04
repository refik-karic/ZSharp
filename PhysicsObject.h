#pragma once

#include "AABB.h"
#include "ZBaseTypes.h"
#include "Vec3.h"
#include "WorldObject.h"

namespace ZSharp {

enum class PhysicsTag {
  Unbound,
  Dynamic,
  Static
};

class PhysicsObject : public WorldObject {
  public:

  AABB& BoundingBox();

  const AABB& BoundingBox() const;

  Vec3& Velocity();

  const Vec3& Velocity() const;

  PhysicsTag& Tag();

  const PhysicsTag& Tag() const;

  private:
  PhysicsTag mTag = PhysicsTag::Unbound;
  Vec3 mVelocity;
  AABB mBoundingBox;
};

}
