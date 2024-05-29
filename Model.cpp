#include "Model.h"

#include "ZAssert.h"
#include "ScopedTimer.h"
#include "PlatformIntrinsics.h"

namespace ZSharp {
Model::Model(const ShadingModeOrder& order, size_t stride)
  : mShadingOrder(order), mStride(stride) {
}

Model::Model(const Model& copy) 
  : mShadingOrder(copy.mShadingOrder), mStride(copy.mStride), mData(copy.mData) {
}

void Model::operator=(const Model& rhs) {
  if (this == &rhs) {
    return;
  }

  mShadingOrder = rhs.mShadingOrder;
  mStride = rhs.mStride;
  mData = rhs.mData;
}

Mesh& Model::operator[](size_t index) {
  ZAssert(index < mData.Size());
  return mData[index];
}

size_t Model::MeshCount() const {
  return mData.Size();
}

void Model::CreateNewMesh() {
  mData.EmplaceBack(mStride);
}

Array<Mesh>& Model::GetMeshData() {
  return mData;
}

const Array<Mesh>& Model::GetMeshData() const {
  return mData;
}

size_t Model::Stride() const {
  return mStride;
}

const ShadingModeOrder& Model::ShadingOrder() const {
  return mShadingOrder;
}

void Model::SetShadingOrder(const ShadingModeOrder& order) {
  mShadingOrder = order;
}

void Model::SetStride(size_t stride) {
  mStride = stride;

  for (Mesh& mesh : mData) {
    mesh.SetNumAttributes(stride);
  }
}

void Model::FillBuffers(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer) const {
  for (const Mesh& mesh : mData) {
    if (mesh.GetTriangleFaceTable().Size() == 0 || mesh.GetVertTables()[0].Size() == 0) {
      continue;
    }

    indexBuffer.CopyInputData(reinterpret_cast<const int32*>(mesh.GetTriangleFaceTable().GetData()), 0, (int32)(mesh.GetTriangleFaceTable().Size() * TRI_VERTS));
    const float* xyzw[16] = { mesh.GetVertTables()[0].GetData(), mesh.GetVertTables()[1].GetData(), mesh.GetVertTables()[2].GetData(), mesh.GetVertTables()[3].GetData() };
    int32 attributes = (int32)mesh.NumAttributes();
    for (int32 i = 0; i < attributes; ++i) {
      xyzw[4 + i] = mesh.GetAttributeTables()[i].GetData();
    }
    vertexBuffer.CopyInputData(xyzw, 0, (int32)(mesh.GetVertTables()[0].Size()), attributes);
  }
}

int32& Model::TextureId() {
  return mTextureId;
}

AABB Model::ComputeBoundingBox() const {
  NamedScopedTimer(ModelComputeAABB);

  float min[4] = { INFINITY, INFINITY, INFINITY, INFINITY };
  float max[4] = { -INFINITY, -INFINITY, -INFINITY, -INFINITY };

  for (Mesh& mesh : mData) {
    Array<float>* meshVerts = mesh.GetVertTables();
    const float* verts[4] = { meshVerts[0].GetData(), meshVerts[1].GetData(), meshVerts[2].GetData(), meshVerts[3].GetData() };
    Unaligned_AABBSOA(verts, meshVerts[0].Size(), min, max);
  }

  AABB aabb(min, max);

  return aabb;
}

}
