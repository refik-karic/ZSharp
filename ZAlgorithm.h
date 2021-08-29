#pragma once

#include <array>

#include "Constants.h"
#include "IndexBuffer.h"
#include "Vec3.h"
#include "VertexBuffer.h"

namespace ZSharp {

class ZAlgorithm final {
  public:
  static bool Inside(const Vec3& point, const Vec3& clipEdge) {
    return ((clipEdge * (point - clipEdge)) <= 0.f);
  }

  static Vec3 GetParametricVector(float point, Vec3 start, Vec3 end) {
    return (start + ((end - start) * point));
  }

  static float ParametricLinePlaneIntersection(Vec3 start, Vec3 end, Vec3 edgeNormal, Vec3 edgePoint) {
    float numerator = edgeNormal * (start - edgePoint);
    float denominator = (edgeNormal * -1.f) * (end - start);
    return (numerator / denominator);
  }

  static std::size_t SutherlandHodgmanClip(std::array<Vec3, 6>& inputVerts, std::size_t numInputVerts, const Vec3& clipEdge) {
    std::size_t numOutputVerts = 0;
    std::array<Vec3, 6> outputVerts;

    for(std::size_t i = 0; i < numInputVerts; ++i) {
      std::size_t nextIndex = (i + 1) % numInputVerts;

      if(!Inside(inputVerts[i], clipEdge) && !Inside(inputVerts[nextIndex], clipEdge)) {
        continue;
      }
      else if(Inside(inputVerts[i], clipEdge) && Inside(inputVerts[nextIndex], clipEdge)) {
        outputVerts[numOutputVerts] = inputVerts[nextIndex];
        ++numOutputVerts;
      }
      else {
        const float parametricValue = ParametricLinePlaneIntersection(inputVerts[i], inputVerts[nextIndex], clipEdge, clipEdge);
        const Vec3 clipPoint = GetParametricVector(parametricValue, inputVerts[i], inputVerts[nextIndex]);

        if(!Inside(inputVerts[i], clipEdge)) {
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

    for(std::size_t i = 0; i < numOutputVerts; ++i) {
      inputVerts[i] = outputVerts[i];
    }

    return numOutputVerts;
  }

  static void CullBackFacingPrimitives(const VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, Vec3 viewer) {
    const std::size_t stride = vertexBuffer.GetHomogenizedStride();
    for(std::size_t i = indexBuffer.GetWorkingSize(); i >= Constants::TRI_VERTS; i -= Constants::TRI_VERTS) {
      const float* v1 = vertexBuffer.GetInputData(indexBuffer[i - 3], stride);
      const float* v2 = vertexBuffer.GetInputData(indexBuffer[i - 2], stride);
      const float* v3 = vertexBuffer.GetInputData(indexBuffer[i - 1], stride);
      const Vec3& firstEdge = *(reinterpret_cast<const Vec3*>(v1));
      const Vec3& secondEdge = *(reinterpret_cast<const Vec3*>(v2));
      const Vec3& thirdEdge = *(reinterpret_cast<const Vec3*>(v3));
      Vec3 p1p0 = secondEdge - firstEdge;
      Vec3 p2p1 = thirdEdge - secondEdge;
      Vec3 triangleNormal = Vec3::Cross(p1p0, p2p1);
      float dotResult = (viewer - secondEdge) * triangleNormal;
      if(dotResult <= 0.f) {
        indexBuffer.RemoveTriangle((i / Constants::TRI_VERTS) - 1);
      }
    }
  }

  private:
};

}
