#pragma once

#include "ZBaseTypes.h"

#include "PhysicsObject.h"
#include "Vec3.h"

namespace ZSharp {

extern const float GravityPerSecond;
extern const float IntervalEpsilon;

void CorrectOverlappingObjects(PhysicsObject& a, PhysicsObject& b);

float MinDistanceForTime(PhysicsObject& a, PhysicsObject& b, float t);

float MaxMovementForTime(PhysicsObject& object, float t0, float t1);

bool StaticContinuousTest(PhysicsObject& a, PhysicsObject& b, float t0, float t1, float& timeOfImpact);

bool DynamicDynamicIntersectionTest(PhysicsObject& a, PhysicsObject& b, float& timeOfImpact);

bool GJKTest(PhysicsObject& a, PhysicsObject& b, float& timeOfImpact);

// TODO: We need functions for collision handling.

}
