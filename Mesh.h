#pragma once

#include "ZBaseTypes.h"

#include "Array.h"
#include "Triangle.h"

namespace ZSharp {

class Mesh final {
  public:

  Mesh() = default;

  Mesh(size_t stride);

  Mesh(size_t numVerts, size_t stride, size_t numTriangleFaces);

  Mesh(const Mesh& copy);
  
  void operator=(const Mesh& rhs) {
    if (this == &rhs) {
      return;
    }

    mStride = rhs.mStride;
    mVertTable = rhs.mVertTable;
    mTriangleFaceTable = rhs.mTriangleFaceTable;
  }

  void Resize(size_t vertexLength, size_t faceTableLength);

  void SetData(const float* vertData, size_t index, size_t numBytes);

  void SetTriangle(const Triangle& triangle, size_t index);

  Array<float>& GetVertTable();

  const Array<float>& GetVertTable() const;

  size_t Stride() const;

  void SetStride(size_t stride);

  Array<Triangle>& GetTriangleFaceTable();

  const Array<Triangle>& GetTriangleFaceTable() const;

  private:
  size_t mStride = 0;
  Array<float> mVertTable;
  Array<Triangle> mTriangleFaceTable;
};
}
