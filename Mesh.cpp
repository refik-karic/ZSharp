#include "Mesh.h"

#include "PlatformIntrinsics.h"
#include "ScopedTimer.h"

#include <cstring>

namespace ZSharp {

Mesh::Mesh(size_t numVerts, size_t numTriangleFaces) {
  mVertTable.Resize(numVerts);
  mTriangleFaceTable.Resize(numTriangleFaces);
}

Mesh::Mesh(const Mesh& copy) 
  : mTextureId(copy.mTextureId), mAlbedoTexture(copy.mAlbedoTexture), mShader(copy.mShader), mVertTable(copy.mVertTable), mTriangleFaceTable(copy.mTriangleFaceTable) {
}

void Mesh::operator=(const Mesh& rhs) {
  if (this == &rhs) {
    return;
  }

  mTextureId = rhs.mTextureId;
  mAlbedoTexture = rhs.mAlbedoTexture;
  mShader = rhs.mShader;
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

Array<Triangle>& Mesh::GetTriangleFaceTable() {
  return mTriangleFaceTable;
}

const Array<Triangle>& Mesh::GetTriangleFaceTable() const {
  return mTriangleFaceTable;
}

const ShaderDefinition& Mesh::GetShader() const {
  return mShader;
}

void Mesh::SetShader(const ShaderDefinition& shader) {
  mShader = shader;
}

int32& Mesh::TextureId() {
  return mTextureId;
}

String& Mesh::AlbedoTexture() {
  return mAlbedoTexture;
}

size_t Mesh::Stride() const {
  // XYZW + attributes
  return 4 + mShader.GetAttributeStride();
}

void Mesh::Serialize(ISerializer& serializer) {
  mAlbedoTexture.Serialize(serializer);
  mShader.Serialize(serializer);
  mVertTable.Serialize(serializer);
  mTriangleFaceTable.Serialize(serializer);
}

void Mesh::Deserialize(IDeserializer& deserializer) {
  mAlbedoTexture.Deserialize(deserializer);
  mShader.Deserialize(deserializer);
  mVertTable.Deserialize(deserializer);
  mTriangleFaceTable.Deserialize(deserializer);
}

}
