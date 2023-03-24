#include "World.h"

#include "AssetLoader.h"

namespace ZSharp {
World::World() {
}

void World::LoadModel(FileString& path, ShadingModeOrder order, size_t stride) {
  {
    Model model(order, stride);
    mActiveModels.PushBack(model);
  }

  {
    VertexBuffer vertBuffer;
    mVertexBuffers.PushBack(vertBuffer);
  }

  {
    IndexBuffer indexBuffer;
    mIndexBuffers.PushBack(indexBuffer);
  }
  
  Model& cachedModel = mActiveModels[mActiveModels.Size() - 1];
  LoadModelOBJ(path, cachedModel);

  VertexBuffer& cachedVertBuffer = mVertexBuffers[mVertexBuffers.Size() - 1];
  IndexBuffer& cachedIndexBuffer = mIndexBuffers[mIndexBuffers.Size() - 1];

  size_t indexBufSize = 0;
  size_t vertBufSize = 0;
  size_t vertStride = 0;
  for (Mesh& mesh : cachedModel.GetMeshData()) {
    indexBufSize += (mesh.GetTriangleFaceTable().Size() * TRI_VERTS);
    vertBufSize += mesh.GetVertTable().Size();
    vertStride = mesh.GetStride();
  }

  cachedIndexBuffer.Resize(indexBufSize);
  cachedVertBuffer.Resize(vertBufSize, vertStride);
}

void World::DebugLoadTriangle(const float* v1, const float* v2, const float* v3, ShadingModeOrder order, size_t stride)
{
  {
    Model model(order, stride);
    mActiveModels.PushBack(model);
  }

  {
    VertexBuffer vertBuffer;
    mVertexBuffers.PushBack(vertBuffer);
  }

  {
    IndexBuffer indexBuffer;
    mIndexBuffers.PushBack(indexBuffer);
  }

  Model& cachedModel = mActiveModels[mActiveModels.Size() - 1];
  cachedModel.CreateNewMesh();
  
  {
    const size_t strideBytes = stride * sizeof(float);
    Mesh& firstMesh = cachedModel[0];
    size_t vertSize = 3 * stride;
    size_t faceSize = 1;
    firstMesh.Resize(vertSize, faceSize);
    firstMesh.SetData(v1, 0, strideBytes);
    firstMesh.SetData(v2, stride, strideBytes);
    firstMesh.SetData(v3, stride * 2, strideBytes);
    Triangle triangle(0, 1, 2);
    firstMesh.SetTriangle(triangle, 0);
  }

  VertexBuffer& cachedVertBuffer = mVertexBuffers[mVertexBuffers.Size() - 1];
  IndexBuffer& cachedIndexBuffer = mIndexBuffers[mIndexBuffers.Size() - 1];

  size_t indexBufSize = 0;
  size_t vertBufSize = 0;
  for (Mesh& mesh : cachedModel.GetMeshData()) {
    indexBufSize += (mesh.GetTriangleFaceTable().Size() * TRI_VERTS);
    vertBufSize += mesh.GetVertTable().Size();
  }

  cachedIndexBuffer.Resize(indexBufSize);
  cachedVertBuffer.Resize(vertBufSize, stride);
}

size_t World::GetTotalModels() const {
  return mActiveModels.Size();
}

Array<Model>& World::GetModels() {
  return mActiveModels;
}

Array<VertexBuffer>& World::GetVertexBuffers() {
  return mVertexBuffers;
}

Array<IndexBuffer>& World::GetIndexBuffers() {
  return mIndexBuffers;
}
}
