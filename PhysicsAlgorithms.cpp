#include "PhysicsAlgorithms.h"

namespace ZSharp {

// TODO: This is arbitrarily assigned. We need a cohesive unit system for this to make sense.
const float GravityPerSecond = -0.0001f;

// TODO: Does this value make sense?
const float IntervalEpsilon = 1.0e-4f;

void CorrectOverlappingObjects(PhysicsObject& a, PhysicsObject& b) {
  (void)a;
  (void)b;

  // TODO: We need to apply a realistic counter force here.
  //  Depending on the mass and object properties, we need to handle elasticity.
  const float hackyCounterForce = fabsf(a.Velocity()[1]) * 0.5f;
  a.Velocity() = Vec3(0.f, hackyCounterForce, 0.f);
}

float MinDistanceForTime(PhysicsObject& a, PhysicsObject& b, float t) {
  Vec3 aPos(a.TransformedAABB().MinBounds() + (a.Velocity() * t));
  Vec3 bPos(b.TransformedAABB().MinBounds() + (b.Velocity() * t));
  return (aPos - bPos).Length();
}

float MaxMovementForTime(PhysicsObject& object, float t0, float t1) {
  const Vec3 scaledStart(object.Velocity() * t0);
  const Vec3 scaledEnd(object.Velocity() * t1);

  const Vec3 distance(scaledEnd - scaledStart);

  return distance.Length();
}

bool StaticContinuousTest(PhysicsObject& a, PhysicsObject& b, float t0, float t1, float& timeOfImpact) {
  /*
    Perform a recursive binary search over the movement for the current timestep.

    If the objects never reach each other for the timestep we can early out.
    Otherwise, we check the first half of the space and then the second.
    Eventually A will come very close to B but not actually collide with it.
    At that point, we can use something like GJK to approximate an impact point and handle the collision.
  */
  
  float maxMoveA = MaxMovementForTime(a, t0, t1);

  // NOTE: In a dynamic/dynamic case we must consider the other moving object.
  //  Since we're dealing with dynamic/static here, we can assume B is fixed.
  //float maxMoveB = MaxDistanceForTime(b, t0, t1);
  float maxMoveB = 0.f;

  float maxMoveDistance = maxMoveA + maxMoveB;

  float minDistanceStart = MinDistanceForTime(a, b, t0);
  if (minDistanceStart > maxMoveDistance) {
    return false;
  }

  float minDistanceEnd = MinDistanceForTime(a, b, t1);
  if (minDistanceEnd > maxMoveDistance) {
    return false;
  }

  if ((t1 - t0) < IntervalEpsilon) {
    timeOfImpact = t0;
    return true;
  }

  float midTime = (t0 + t1) * 0.5f;

  if (StaticContinuousTest(a, b, t0, midTime, timeOfImpact)) {
    return true;
  }
  else {
    return StaticContinuousTest(a, b, midTime, t1, timeOfImpact);
  }
}

bool DynamicDynamicIntersectionTest(PhysicsObject& a, PhysicsObject& b, float& timeOfImpact) {
  (void)a;
  (void)b;
  (void)timeOfImpact;

  return false;
}

bool GJKTest(PhysicsObject& a, PhysicsObject& b, float& timeOfImpact) {
  (void)a;
  (void)b;
  (void)timeOfImpact;

  return false;
}

}
