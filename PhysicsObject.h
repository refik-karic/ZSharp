#pragma once

#include "AABB.h"
#include "Delegate.h"
#include "Vec3.h"
#include "WorldObject.h"
#include "ZBaseTypes.h"

namespace ZSharp {

enum class PhysicsTag {
  Unbound,
  Dynamic,
  Static,
  Player
};

class PhysicsObject : public WorldObject {
  public:

  float& Mass();

  const float& Mass() const;

  AABB& BoundingBox();

  const AABB& BoundingBox() const;

  AABB TransformedAABB() const;

  Vec3& Velocity();

  const Vec3& Velocity() const;

  PhysicsTag& Tag();

  const PhysicsTag& Tag() const;

  Delegate<PhysicsObject*> OnCollisionStartDelegate;
  Delegate<PhysicsObject*> OnCollisionEndDelegate;

  protected:
  PhysicsTag mTag = PhysicsTag::Unbound;
  Vec3 mVelocity;
  AABB mBoundingBox;
  float mMass = 1.f;
};

}
