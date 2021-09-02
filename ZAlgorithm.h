#pragma once

#include <array>

#include "IndexBuffer.h"
#include "Vec3.h"
#include "VertexBuffer.h"

namespace ZSharp {

bool InsidePlane(const Vec3& point, const Vec3& clipEdge);

Vec3 GetParametricVector(float point, Vec3 start, Vec3 end);

float ParametricLinePlaneIntersection(Vec3 start, Vec3 end, Vec3 edgeNormal, Vec3 edgePoint);

std::size_t SutherlandHodgmanClip(std::array<Vec3, 6>& inputVerts, std::size_t numInputVerts, const Vec3& clipEdge);

void CullBackFacingPrimitives(const VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, Vec3 viewer);

}
