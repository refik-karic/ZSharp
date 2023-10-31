#pragma once

#include "IndexBuffer.h"
#include "Vec3.h"
#include "VertexBuffer.h"
#include "PlatformDefines.h"
#include "AABB.h"
#include "ZColor.h"

namespace ZSharp {

FORCE_INLINE bool InsidePlane(const float point[3], const float clipEdge[3], const float normal[3]);

void GetParametricVector4D(const float point, const float start[4], const float end[4], float outVec[4]);

float ParametricLinePlaneIntersection(const float start[3], const float end[3], const float edgeNormal[3], const float edgePoint[3]);

void ClipTrianglesNearPlane(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, float nearClipZ);

void ClipTriangles(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer);

size_t SutherlandHodgmanClip(float* inputVerts, const size_t numInputVerts, const float clipEdge[3], const float edgeNormal[3], size_t stride);

void CullBackFacingPrimitives(const VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, const Vec3& viewer);

void TriangulateAABB(const AABB& aabb, VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer);

void TriangulateAABBWithColor(const AABB& aabb, VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, const ZColor& color);

}
