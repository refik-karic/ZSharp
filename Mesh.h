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

  Mesh(std::size_t numVerts, std::size_t numTriangleFaces);

  Mesh(const Mesh& copy);
  
  void operator=(const Mesh& rhs) {
    if (this == &rhs) {
      return;
    }

    mVertTable = rhs.mVertTable;
    mTriangleFaceTable = rhs.mTriangleFaceTable;
  }

  void SetData(const float* vertData, std::size_t numVerts, std::size_t numTriangleFaces);

  void SetVertex(const Vec4& vertex, std::size_t index, std::size_t numElements);

  void SetTriangle(const std::array<size_t, 3>& triangleFaceData, std::size_t index);

  std::vector<float>& GetVertTable();

  const std::vector<float>& GetVertTable() const;

  std::vector<Triangle>& GetTriangleFaceTable();

  const std::vector<Triangle>& GetTriangleFaceTable() const;

  private:
  std::vector<float> mVertTable;
  std::vector<Triangle> mTriangleFaceTable;
};
}
