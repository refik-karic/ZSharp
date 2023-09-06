#include "Model.h"

#include "ZAssert.h"
#include "Triangle.h"
#include "PNG.h"
#include "ScopedTimer.h"
#include "PlatformIntrinsics.h"

#define FAST_LOAD 1

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
  Mesh mesh(mStride);
  mData.PushBack(mesh);
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
  for (const Mesh& mesh : mData) {
    if (mesh.GetTriangleFaceTable().Size() == 0 || mesh.GetVertTable().Size() == 0) {
      continue;
    }

#if FAST_LOAD
    indexBuffer.CopyInputData(reinterpret_cast<const size_t*>(mesh.GetTriangleFaceTable().GetData()), 0, mesh.GetTriangleFaceTable().Size() * TRI_VERTS);
    vertexBuffer.CopyInputData(mesh.GetVertTable().GetData(), 0, mesh.GetVertTable().Size());
#else
    for (size_t i = 0; i < mesh.GetTriangleFaceTable().size(); ++i) {
      const Triangle& triangle = mesh.GetTriangleFaceTable()[i];
      indexBuffer.CopyInputData(triangle.GetData(), i * TRI_VERTS, TRI_VERTS);
    }

    for (size_t i = 0; i < mesh.GetVertTable().size(); ++i) {
      float vert = mesh.GetVertTable()[i];
      vertexBuffer.CopyInputData(&vert, i, 1);
    }
#endif
  }
}

void Model::BindTexture(uint8* data, size_t width, size_t height, size_t channels) {
  mTexture.Assign(data, channels, width, height);
}

Texture& Model::GetTexture() {
  return mTexture;
}

AABB Model::ComputeBoundingBox() const {
  NamedScopedTimer(ComputeAABB);

  float min[4] = {};
  float max[4] = {};

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
