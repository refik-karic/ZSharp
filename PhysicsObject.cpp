#include "PhysicsObject.h"

namespace ZSharp {
float& PhysicsObject::Mass() {
  return mMass;
}

const float& PhysicsObject::Mass() const {
  return mMass;
}

AABB& PhysicsObject::BoundingBox() {
  return mBoundingBox;
}

const AABB& PhysicsObject::BoundingBox() const {
  return mBoundingBox;
}

AABB PhysicsObject::TransformedAABB() const {
  return AABB::TransformAndRealign(mBoundingBox, ObjectTransform());
}

Vec3& PhysicsObject::Velocity() {
  return mVelocity;
}

const Vec3& PhysicsObject::Velocity() const {
  return mVelocity;
}

PhysicsTag& PhysicsObject::Tag() {
  return mTag;
}

const PhysicsTag& PhysicsObject::Tag() const {
  return mTag;
}

}
