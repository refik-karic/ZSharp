#pragma once

#include "IndexBuffer.h"
#include "Vec3.h"
#include "VertexBuffer.h"
#include "PlatformDefines.h"
#include "AABB.h"
#include "ZColor.h"

namespace ZSharp {

bool InsidePlane(const float point[4], const float clipEdge[4], const float normal[4]);

void GetParametricVector4D(const float point, const float start[4], const float end[4], float outVec[4]);

float ParametricLinePlaneIntersection(const float start[4], const float end[4], const float edgeNormal[4], const float edgePoint[4]);

void ClipTrianglesNearPlane(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, float nearClipZ);

void ClipTriangles(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer);

int32 SutherlandHodgmanClip(float* inputVerts, const int32 numInputVerts, const float clipEdge[4], const float edgeNormal[4], int32 stride);

void CullBackFacingPrimitives(const VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, const Vec3& viewer);

void TriangulateAABB(const AABB& aabb, VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer);

void TriangulateAABBWithColor(const AABB& aabb, VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, const ZColor& color);

}
