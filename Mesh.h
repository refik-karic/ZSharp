#pragma once

#include <cstddef>
#include <array>
#include <vector>

#include "Triangle.h"
#include "Vec4.h"

namespace ZSharp {

class Mesh final {
  public:

  Mesh();

  Mesh(size_t numVerts, size_t numTriangleFaces);

  Mesh(const Mesh& copy);
  
  void operator=(const Mesh& rhs) {
    if (this == &rhs) {
      return;
    }

    mVertTable = rhs.mVertTable;
    mTriangleFaceTable = rhs.mTriangleFaceTable;
  }

  void SetData(const float* vertData, size_t numVerts, size_t numTriangleFaces);

  void SetVertex(const Vec4& vertex, size_t index, size_t numElements);

  void SetTriangle(const std::array<size_t, 3>& triangleFaceData, size_t index);

  std::vector<float>& GetVertTable();

  const std::vector<float>& GetVertTable() const;

  std::vector<Triangle>& GetTriangleFaceTable();

  const std::vector<Triangle>& GetTriangleFaceTable() const;

  private:
  std::vector<float> mVertTable;
  std::vector<Triangle> mTriangleFaceTable;
};
}
