#include "Mesh.h"

#include "PlatformIntrinsics.h"

#include <cstring>

namespace ZSharp {
Mesh::Mesh(size_t stride)
  : mStride(stride) {

}

Mesh::Mesh(size_t numVerts, size_t stride, size_t numTriangleFaces) :
  mStride(stride) {
  mVertTable.Resize(numVerts);
  mTriangleFaceTable.Resize(numTriangleFaces);
}

Mesh::Mesh(const Mesh& copy) 
  : mStride(copy.mStride), mVertTable(copy.mVertTable), mTriangleFaceTable(copy.mTriangleFaceTable) {
}

void Mesh::operator=(const Mesh& rhs) {
  if (this == &rhs) {
    return;
  }

  mStride = rhs.mStride;
  mVertTable = rhs.mVertTable;
  mTriangleFaceTable = rhs.mTriangleFaceTable;
}

void Mesh::Resize(size_t vertexLength, size_t faceTableLength) {
  mVertTable.Resize(vertexLength);
  mTriangleFaceTable.Resize(faceTableLength);
}

void Mesh::SetData(const float* vertData, size_t index, size_t numBytes) {
  Aligned_Memcpy(mVertTable.GetData() + index, vertData, numBytes);
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

size_t Mesh::Stride() const {
  return mStride;
}

void Mesh::SetStride(size_t stride) {
  mStride = stride;
}

Array<Triangle>& Mesh::GetTriangleFaceTable() {
  return mTriangleFaceTable;
}

const Array<Triangle>& Mesh::GetTriangleFaceTable() const {
  return mTriangleFaceTable;
}
}
