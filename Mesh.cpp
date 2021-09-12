#include "Mesh.h"

namespace ZSharp {
Mesh::Mesh() {

}

Mesh::Mesh(std::size_t numVerts, std::size_t numTriangleFaces) {
  mVertTable.resize(numVerts);
  mTriangleFaceTable.resize(numTriangleFaces);
}

Mesh::Mesh(const Mesh& copy) {
  *this = copy;
}

void Mesh::SetData(const float* vertData, std::size_t numVerts, std::size_t numTriangleFaces) {
  mVertTable.resize(numVerts);

  for (std::size_t i = 0; i < numVerts; ++i) {
    mVertTable[i] = *(vertData + i);
  }

  mTriangleFaceTable.resize(numTriangleFaces);
}

void Mesh::SetVertex(const Vec4& vertex, std::size_t index, std::size_t numElements) {
  std::size_t vertIndex = 0;
  for (std::size_t i = index; i < index + numElements; ++i) {
    mVertTable[i] = vertex[vertIndex];
    ++vertIndex;
  }
}

void Mesh::SetTriangle(const std::array<size_t, 3>& triangleFaceData, std::size_t index) {
  Triangle& triangle = mTriangleFaceTable[index];
  triangle[0] = triangleFaceData[0];
  triangle[1] = triangleFaceData[1];
  triangle[2] = triangleFaceData[2];
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
