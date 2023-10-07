#include "PhysicsObject.h"

namespace ZSharp {

Vec3& PhysicsObject::Velocity() {
  return mVelocity;
}

const Vec3& PhysicsObject::Velocity() const {
  return mVelocity;
}

}
