#pragma once

#include "IndexBuffer.h"
#include "FixedArray.h"
#include "Vec3.h"
#include "VertexBuffer.h"

namespace ZSharp {

bool InsidePlane(const Vec3& point, const Vec3& clipEdge);

Vec3 GetParametricVector(const float point, const Vec3& start, const Vec3& end);

float ParametricLinePlaneIntersection(const Vec3& start, const Vec3& end, const Vec3& edgeNormal, const Vec3& edgePoint);

void ClipTriangles(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer);

size_t SutherlandHodgmanClip(float* inputVerts, const size_t numInputVerts, const Vec3& clipEdge);

void CullBackFacingPrimitives(const VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, const Vec3& viewer);

}
