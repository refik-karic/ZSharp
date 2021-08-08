#pragma once

#include <cstddef>
#include <array>
#include <vector>

#include "Triangle.h"
#include "Vec4.h"

namespace ZSharp {

template<typename T>
class Mesh final {
  public:

  Mesh() {

  }

  Mesh(std::size_t numVerts, std::size_t numTriangleFaces) {
    mVertTable.resize(numVerts);
    mTriangleFaceTable.resize(numTriangleFaces);
  }

  Mesh(const Mesh<T>& copy) {
    if (this == &copy) {
      return;
    }

    *this = copy;
  }
  
  void operator=(const Mesh<T>& rhs) {
    if (this == &rhs) {
      return;
    }

    mVertTable = rhs.mVertTable;
    mTriangleFaceTable = rhs.mTriangleFaceTable;
  }

  void SetData(const T* vertData, std::size_t numVerts, std::size_t numTriangleFaces) {
    mVertTable.resize(numVerts);

    for (std::size_t i = 0; i < numVerts; ++i) {
      mVertTable[i] = *(vertData + i);
    }

    mTriangleFaceTable.resize(numTriangleFaces);
  }

  void SetVertex(const Vec4<T>& vertex, std::size_t index, std::size_t numElements) {
    std::size_t vertIndex = 0;
    for (std::size_t i = index; i < index + numElements; ++i) {
      mVertTable[i] = vertex[vertIndex];
      ++vertIndex;
    }
  }

  void SetTriangle(const std::array<size_t, 3>& triangleFaceData, std::size_t index) {
    Triangle<T>& triangle = mTriangleFaceTable[index];
    triangle[0] = triangleFaceData[0];
    triangle[1] = triangleFaceData[1];
    triangle[2] = triangleFaceData[2];
  }

  std::vector<T>& GetVertTable() {
    return mVertTable;
  }

  const std::vector<T>& GetVertTable() const {
    return mVertTable;
  }

  std::vector<Triangle<T>>& GetTriangleFaceTable() {
    return mTriangleFaceTable;
  }

  const std::vector<Triangle<T>>& GetTriangleFaceTable() const {
    return mTriangleFaceTable;
  }

  private:
  std::vector<T> mVertTable;
  std::vector<Triangle<T>> mTriangleFaceTable;
};
}
