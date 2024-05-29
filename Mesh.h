#pragma once

#include "ZBaseTypes.h"

#include "Array.h"
#include "Triangle.h"

namespace ZSharp {

class Mesh final {
  public:

  Mesh() = default;

  Mesh(size_t numAttributes);

  Mesh(size_t numVerts, size_t numAttributes, size_t numTriangleFaces);

  Mesh(const Mesh& copy);
  
  void operator=(const Mesh& rhs);

  void Resize(size_t vertexLength, size_t faceTableLength);

  void SetDataSOA(const float** data, size_t index, size_t numBytes);

  void SetDataAOS(const float* data, size_t index, size_t numBytes, size_t stride);

  void SetTriangle(const Triangle& triangle, size_t index);

  Array<float>* GetVertTables();

  const Array<float>* GetVertTables() const;

  Array<Array<float>>& GetAttributeTables();

  const Array<Array<float>>& GetAttributeTables() const;

  size_t NumAttributes() const;

  void SetNumAttributes(size_t numAttributes);

  Array<Triangle>& GetTriangleFaceTable();

  const Array<Triangle>& GetTriangleFaceTable() const;

  private:
  size_t mNumAttributes = 0;
  Array<float> mVertTables[4];
  Array<Array<float>> mAttributeTables;
  Array<Triangle> mTriangleFaceTable;
};
}
