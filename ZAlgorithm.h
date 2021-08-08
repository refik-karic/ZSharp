#pragma once

#include <array>

#include "Constants.h"
#include "IndexBuffer.h"
#include "Vec3.h"
#include "VertexBuffer.h"

namespace ZSharp {

template <typename T>
class ZAlgorithm final {
  public:
  static bool Inside(const Vec3<T>& point, const Vec3<T>& clipEdge) {
    return ((clipEdge * (point - clipEdge)) <= static_cast<T>(0));
  }

  static Vec3<T> GetParametricVector(T point, Vec3<T> start, Vec3<T> end) {
    return (start + ((end - start) * point));
  }

  static T ParametricLinePlaneIntersection(Vec3<T> start, Vec3<T> end, Vec3<T> edgeNormal, Vec3<T> edgePoint) {
    T numerator = edgeNormal * (start - edgePoint);
    T denominator = (edgeNormal * static_cast<T>(-1)) * (end - start);
    return (numerator / denominator);
  }

  static std::size_t SutherlandHodgmanClip(std::array<Vec3<T>, 6>& inputVerts, std::size_t numInputVerts, const Vec3<T>& clipEdge) {
    std::size_t numOutputVerts = 0;
    std::array<Vec3<T>, 6> outputVerts;

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
        const T parametricValue = ParametricLinePlaneIntersection(inputVerts[i], inputVerts[nextIndex], clipEdge, clipEdge);
        const Vec3<T> clipPoint = GetParametricVector(parametricValue, inputVerts[i], inputVerts[nextIndex]);

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

  static void CullBackFacingPrimitives(const VertexBuffer<T>& vertexBuffer, IndexBuffer& indexBuffer, Vec3<T> viewer) {
    const std::size_t stride = vertexBuffer.GetHomogenizedStride();
    for(std::size_t i = indexBuffer.GetWorkingSize(); i >= Constants::TRI_VERTS; i -= Constants::TRI_VERTS) {
      const T* v1 = vertexBuffer.GetInputData(indexBuffer[i - 3], stride);
      const T* v2 = vertexBuffer.GetInputData(indexBuffer[i - 2], stride);
      const T* v3 = vertexBuffer.GetInputData(indexBuffer[i - 1], stride);
      const Vec3<T>& firstEdge = *(reinterpret_cast<const Vec3<T>*>(v1));
      const Vec3<T>& secondEdge = *(reinterpret_cast<const Vec3<T>*>(v2));
      const Vec3<T>& thirdEdge = *(reinterpret_cast<const Vec3<T>*>(v3));
      Vec3<T> p1p0 = secondEdge - firstEdge;
      Vec3<T> p2p1 = thirdEdge - secondEdge;
      Vec3<T> triangleNormal = Vec3<T>::Cross(p1p0, p2p1);
      T dotResult = (viewer - secondEdge) * triangleNormal;
      if(dotResult <= static_cast<T>(0)) {
        indexBuffer.RemoveTriangle((i / Constants::TRI_VERTS) - 1);
      }
    }
  }

  private:
};

}
