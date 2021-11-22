#include "ZAlgorithm.h"

#include "Constants.h"
#include "UtilMath.h"

#include <cassert>

namespace ZSharp {
bool InsidePlane(const Vec3& point, const Vec3& clipEdge) {
  return clipEdge * (point - clipEdge) < 0.f;
}

Vec3 GetParametricVector(const float point, const Vec3& start, const Vec3& end) {
  return (start + ((end - start) * point));
}

float ParametricLinePlaneIntersection(const Vec3& start, const Vec3& end, const Vec3& edgeNormal, const Vec3& edgePoint) {
  float numerator = edgeNormal * (start - edgePoint);
  float denominator = (-edgeNormal) * (end - start);
  return (numerator / denominator);
}

size_t SutherlandHodgmanClip(std::array<Vec3, 6>& inputVerts, const size_t numInputVerts, const Vec3& clipEdge) {
  size_t numOutputVerts = 0;
  std::array<Vec3, 6> outputVerts;

  for (size_t i = 0; i < numInputVerts; ++i) {
    size_t nextIndex = (i + 1) % numInputVerts;

    bool p0Inside = InsidePlane(inputVerts[i], clipEdge);
    bool p1Inside = InsidePlane(inputVerts[nextIndex], clipEdge);

    if (!p0Inside && !p1Inside) {
      continue;
    }
    else if (p0Inside && p1Inside) {
      outputVerts[numOutputVerts] = inputVerts[nextIndex];
      ++numOutputVerts;
    }
    else {
      const float parametricValue = ParametricLinePlaneIntersection(inputVerts[i], inputVerts[nextIndex], clipEdge, clipEdge);
      const Vec3 clipPoint(GetParametricVector(parametricValue, inputVerts[i], inputVerts[nextIndex]));

      if (!p0Inside) {
        outputVerts[numOutputVerts] = clipPoint;
        ++numOutputVerts;
        outputVerts[numOutputVerts] = inputVerts[nextIndex];
        ++numOutputVerts;
      }
      else {
        outputVerts[numOutputVerts] = clipPoint;
        ++numOutputVerts;
      }
    }
  }

  for (size_t i = 0; i < numOutputVerts; ++i) {
    inputVerts[i] = outputVerts[i];
  }

  return numOutputVerts;
}

void CullBackFacingPrimitives(const VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, const Vec3& viewer) {
  assert((indexBuffer.GetIndexSize() % TRI_VERTS) == 0);
  
  for (size_t i = indexBuffer.GetIndexSize(); i >= TRI_VERTS; i -= TRI_VERTS) {
    size_t i1 = indexBuffer[i - 3];
    size_t i2 = indexBuffer[i - 2];
    size_t i3 = indexBuffer[i - 1];
    const float* v1 = vertexBuffer[indexBuffer[i1]];
    const float* v2 = vertexBuffer[indexBuffer[i2]];
    const float* v3 = vertexBuffer[indexBuffer[i3]];
    const Vec3& firstEdge = *(reinterpret_cast<const Vec3*>(v1));
    const Vec3& secondEdge = *(reinterpret_cast<const Vec3*>(v2));
    const Vec3& thirdEdge = *(reinterpret_cast<const Vec3*>(v3));
    const Vec3 p1p0(secondEdge - firstEdge);
    const Vec3 p2p1(thirdEdge - secondEdge);
    const Vec3 triangleNormal(p1p0.Cross(p2p1));
    float dotResult = (viewer - secondEdge) * triangleNormal;
    if (FloatLessThanEqual(dotResult, 0.f)) {
      indexBuffer.RemoveTriangle(i / TRI_VERTS);
    }
  }
}
}
