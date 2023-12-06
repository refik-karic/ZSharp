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

bool GJKTestAABB(PhysicsObject& a, PhysicsObject& b, float t, Vec3 aPoints[8], Vec3 bPoints[8], size_t simplexIndices[8]);

bool GJKHandleSimplex(Vec3 simplex[4], size_t simplexIndices[8], Vec3& direction, size_t& simplexLength);

Vec3 GJKSupportPoint(const Vec3* verts, size_t length, const Vec3& direction, size_t& index);

Vec3 ClosestPointToLine(const Vec3& point, const Vec3 line[2], float& t);

Vec3 ClosestPointToTriangle(const Vec3& point, const Vec3 triangle[3]);

// TODO: We need functions for collision handling.

}
