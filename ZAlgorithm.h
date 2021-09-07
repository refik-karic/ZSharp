#pragma once

#include <array>

#include "IndexBuffer.h"
#include "Vec3.h"
#include "VertexBuffer.h"

namespace ZSharp {

bool InsidePlane(const Vec3& point, const Vec3& clipEdge);

Vec3 GetParametricVector(const float point, const Vec3& start, const Vec3& end);

float ParametricLinePlaneIntersection(const Vec3& start, const Vec3& end, const Vec3& edgeNormal, const Vec3& edgePoint);

std::size_t SutherlandHodgmanClip(std::array<Vec3, 6>& inputVerts, const std::size_t numInputVerts, const Vec3& clipEdge);

void CullBackFacingPrimitives(const VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, Vec3 viewer);

}
