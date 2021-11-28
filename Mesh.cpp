#include "Mesh.h"

#include "UtilMath.h"

namespace ZSharp {
Mesh::Mesh() {

}

Mesh::Mesh(size_t numVerts, size_t stride, size_t numTriangleFaces) :
  mStride(stride) {
  mVertTable.Resize(numVerts);
  mTriangleFaceTable.Resize(numTriangleFaces);
}

Mesh::Mesh(const Mesh& copy) {
  *this = copy;
}

void Mesh::Resize(size_t vertexLength, size_t stride, size_t faceTableLength) {
  mVertTable.Resize(vertexLength);
  mTriangleFaceTable.Resize(faceTableLength);
  mStride = stride;
}

void Mesh::SetData(const float* vertData, size_t index, size_t numVerts) {
  memcpy(mVertTable.GetData() + index, vertData, numVerts);
}

void Mesh::SetTriangle(const Triangle& triangle, size_t index) {
  mTriangleFaceTable[index] = triangle;
}

Array<float>& Mesh::GetVertTable() {
  return mVertTable;
}

const Array<float>& Mesh::GetVertTable() const {
  return mVertTable;
}

Array<Triangle>& Mesh::GetTriangleFaceTable() {
  return mTriangleFaceTable;
}

const Array<Triangle>& Mesh::GetTriangleFaceTable() const {
  return mTriangleFaceTable;
}
}
