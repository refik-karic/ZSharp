#pragma once

#include <cstddef>

#include "Array.h"
#include "Triangle.h"
#include "Vec4.h"

namespace ZSharp {

class Mesh final {
  public:

  Mesh();

  Mesh(size_t numVerts, size_t stride, size_t numTriangleFaces);

  Mesh(const Mesh& copy);
  
  void operator=(const Mesh& rhs) {
    if (this == &rhs) {
      return;
    }

    mVertTable = rhs.mVertTable;
    mTriangleFaceTable = rhs.mTriangleFaceTable;
  }

  void Resize(size_t vertexLength, size_t stride, size_t faceTableLength);

  void SetData(const float* vertData, size_t index, size_t numVerts);

  void SetTriangle(const Triangle& triangle, size_t index);

  Array<float>& GetVertTable();

  const Array<float>& GetVertTable() const;

  Array<Triangle>& GetTriangleFaceTable();

  const Array<Triangle>& GetTriangleFaceTable() const;

  private:
  size_t mStride = 0;
  Array<float> mVertTable;
  Array<Triangle> mTriangleFaceTable;
};
}
