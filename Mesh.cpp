#include "Mesh.h"

#include "PlatformIntrinsics.h"

#include <cstring>

namespace ZSharp {
Mesh::Mesh(size_t numAttributes)
  : mNumAttributes(numAttributes), mAttributeTables(numAttributes) {
}

Mesh::Mesh(size_t numVerts, size_t numAttributes, size_t numTriangleFaces) :
  mNumAttributes(numAttributes), mAttributeTables(numAttributes), mTriangleFaceTable(numTriangleFaces) {
  mVertTables[0].Resize(numVerts);
  mVertTables[1].Resize(numVerts);
  mVertTables[2].Resize(numVerts);
  mVertTables[3].Resize(numVerts);
}

Mesh::Mesh(const Mesh& copy) 
  : mNumAttributes(copy.mNumAttributes), mAttributeTables(copy.mAttributeTables), mTriangleFaceTable(copy.mTriangleFaceTable) {
  mVertTables[0] = copy.mVertTables[0];
  mVertTables[1] = copy.mVertTables[1];
  mVertTables[2] = copy.mVertTables[2];
  mVertTables[3] = copy.mVertTables[3];
}

void Mesh::operator=(const Mesh& rhs) {
  if (this == &rhs) {
    return;
  }

  mNumAttributes = rhs.mNumAttributes;
  mVertTables[0] = rhs.mVertTables[0];
  mVertTables[1] = rhs.mVertTables[1];
  mVertTables[2] = rhs.mVertTables[2];
  mVertTables[3] = rhs.mVertTables[3];
  mAttributeTables = rhs.mAttributeTables;
  mTriangleFaceTable = rhs.mTriangleFaceTable;
}

void Mesh::Resize(size_t vertexLength, size_t faceTableLength) {
  mVertTables[0].Resize(vertexLength);
  mVertTables[1].Resize(vertexLength);
  mVertTables[2].Resize(vertexLength);
  mVertTables[3].Resize(vertexLength);

  if (mAttributeTables.Size() != mNumAttributes) {
    mAttributeTables.Resize(mNumAttributes);
  }

  for (Array<float>& attributes : mAttributeTables) {
    attributes.Resize(vertexLength);
  }

  if (mTriangleFaceTable.Size() != faceTableLength) {
    mTriangleFaceTable.Resize(faceTableLength);
  }
}

void Mesh::SetDataSOA(const float** data, size_t index, size_t numBytes) {
  Aligned_Memcpy(mVertTables[0].GetData() + index, data[0], numBytes);
  Aligned_Memcpy(mVertTables[1].GetData() + index, data[1], numBytes);
  Aligned_Memcpy(mVertTables[2].GetData() + index, data[2], numBytes);
  Aligned_Memcpy(mVertTables[3].GetData() + index, data[3], numBytes);
  for (size_t i = 0; i < mNumAttributes; ++i) {
    Aligned_Memcpy(mAttributeTables[i].GetData() + index, data[4 + i], numBytes);
  }
}

void Mesh::SetDataAOS(const float* data, size_t index, size_t numBytes, size_t stride) {
  size_t numVerts = numBytes / sizeof(float);

  for (size_t i = 0, j = 0; i < numVerts; i += stride, ++j) {
    mVertTables[0].GetData()[index + j] = data[i];
    mVertTables[1].GetData()[index + j] = data[i + 1];
    mVertTables[2].GetData()[index + j] = data[i + 2];
    mVertTables[3].GetData()[index + j] = data[i + 3];
    for (size_t k = 0; k < stride - 4; ++k) {
      mAttributeTables[k].GetData()[index + j] = data[i + 4 + k];
    }
  }
}

void Mesh::SetTriangle(const Triangle& triangle, size_t index) {
  mTriangleFaceTable[index] = triangle;
}

Array<float>* Mesh::GetVertTables() {
  return mVertTables;
}

const Array<float>* Mesh::GetVertTables() const {
  return mVertTables;
}

Array<Array<float>>& Mesh::GetAttributeTables() {
  return mAttributeTables;
}

const Array<Array<float>>& Mesh::GetAttributeTables() const {
  return mAttributeTables;
}

size_t Mesh::NumAttributes() const {
  return mNumAttributes;
}

void Mesh::SetNumAttributes(size_t numAttributes) {
  mNumAttributes = numAttributes;
}

Array<Triangle>& Mesh::GetTriangleFaceTable() {
  return mTriangleFaceTable;
}

const Array<Triangle>& Mesh::GetTriangleFaceTable() const {
  return mTriangleFaceTable;
}
}
