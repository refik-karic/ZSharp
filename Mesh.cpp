#include "Mesh.h"

namespace ZSharp {
Mesh::Mesh() {

}

Mesh::Mesh(size_t numVerts, size_t stride, size_t numTriangleFaces) :
  mStride(stride) {
  mVertTable.resize(numVerts);
  mTriangleFaceTable.resize(numTriangleFaces);
}

Mesh::Mesh(const Mesh& copy) {
  *this = copy;
}

void Mesh::SetData(const float* vertData, size_t numVerts, size_t stride, size_t numTriangleFaces) {
  mVertTable.resize(numVerts);
  mStride = stride;

  memcpy(mVertTable.data(), vertData, numVerts);
  mTriangleFaceTable.resize(numTriangleFaces);
}

void Mesh::SetVertex(const Vec4& vertex, size_t index, size_t numElements) {
  size_t vertIndex = 0;
  for (size_t i = index; i < index + numElements; ++i) {
    mVertTable[i] = vertex[vertIndex];
    ++vertIndex;
  }
}

void Mesh::SetTriangle(const Triangle& triangle, size_t index) {
  mTriangleFaceTable[index] = triangle;
}

std::vector<float>& Mesh::GetVertTable() {
  return mVertTable;
}

const std::vector<float>& Mesh::GetVertTable() const {
  return mVertTable;
}

std::vector<Triangle>& Mesh::GetTriangleFaceTable() {
  return mTriangleFaceTable;
}

const std::vector<Triangle>& Mesh::GetTriangleFaceTable() const {
  return mTriangleFaceTable;
}
}
