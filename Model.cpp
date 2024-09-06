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
    mesh.SetStride(stride);
  }
}

void Model::FillBuffers(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer) const {
  NamedScopedTimer(FillBuffers);

  for (const Mesh& mesh : mData) {
    if (mesh.GetTriangleFaceTable().Size() == 0 || mesh.GetVertTable().Size() == 0) {
      continue;
    }

    indexBuffer.CopyInputData(reinterpret_cast<const int32*>(mesh.GetTriangleFaceTable().GetData()), 0, (int32)(mesh.GetTriangleFaceTable().Size() * TRI_VERTS));
    vertexBuffer.CopyInputData(mesh.GetVertTable().GetData(), 0, (int32)(mesh.GetVertTable().Size()));
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
    const size_t stride = mesh.Stride();
    const float* vertices = mesh.GetVertTable().GetData();
    const size_t numVertices = mesh.GetVertTable().Size();

    Unaligned_AABB(vertices, numVertices, stride, min, max);
  }

  AABB aabb(min, max);

  return aabb;
}

}
