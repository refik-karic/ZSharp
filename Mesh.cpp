#include "Mesh.h"

#include "UtilMath.h"

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

void Mesh::Resize(size_t vertexLength, size_t stride, size_t faceTableLength) {
  mVertTable.resize(vertexLength);
  mTriangleFaceTable.resize(faceTableLength);
  mStride = stride;
}

void Mesh::SetData(const float* vertData, size_t index, size_t numVerts) {
  memcpy(mVertTable.data() + index, vertData, numVerts);
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
