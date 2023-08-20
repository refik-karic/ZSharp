#pragma once

#include "IndexBuffer.h"
#include "FixedArray.h"
#include "Vec3.h"
#include "Vec4.h"
#include "VertexBuffer.h"

namespace ZSharp {

bool InsidePlane(const float point[3], const float clipEdge[3], const float normal[3]);

Vec3 GetParametricVector3D(const float point, const Vec3& start, const Vec3& end);

void GetParametricVector4D(const float point, const float start[4], const float end[4], float outVec[4]);

float ParametricLinePlaneIntersection(const float start[3], const float end[3], const float edgeNormal[3], const float edgePoint[3]);

void ClipTrianglesNearPlane(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, float nearClipZ);

void ClipTriangles(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer);

size_t SutherlandHodgmanClip(float* inputVerts, const size_t numInputVerts, const float clipEdge[3], const float edgeNormal[3], size_t stride);

void CullBackFacingPrimitives(const VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, const Vec3& viewer);

}
