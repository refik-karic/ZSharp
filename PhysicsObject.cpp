#include "PhysicsObject.h"

namespace ZSharp {

AABB& PhysicsObject::BoundingBox() {
  return mBoundingBox;
}

const AABB& PhysicsObject::BoundingBox() const {
  return mBoundingBox;
}

Vec3& PhysicsObject::Velocity() {
  return mVelocity;
}

const Vec3& PhysicsObject::Velocity() const {
  return mVelocity;
}

}
