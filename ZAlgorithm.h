#pragma once

#include "IndexBuffer.h"
#include "FixedArray.h"
#include "Vec3.h"
#include "Vec4.h"
#include "VertexBuffer.h"

namespace ZSharp {

bool InsidePlane(const Vec3& point, const Vec3& clipEdge, const Vec3& normal);

Vec3 GetParametricVector3D(const float point, const Vec3& start, const Vec3& end);

Vec4 GetParametricVector4D(const float point, const Vec4& start, const Vec4& end);

float ParametricLinePlaneIntersection(const Vec3& start, const Vec3& end, const Vec3& edgeNormal, const Vec3& edgePoint);

void ClipTrianglesNearPlane(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, float nearClipZ);

void ClipTriangles(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer);

size_t SutherlandHodgmanClip(float* inputVerts, const size_t numInputVerts, const Vec3& clipEdge, const Vec3& edgeNormal, size_t stride);

void CullBackFacingPrimitives(const VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, const Vec3& viewer);

}
