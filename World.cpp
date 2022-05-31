#include "World.h"

#include "AssetLoader.h"

namespace ZSharp {
World::World() {
}

void World::LoadModel(FileString& path) {
  {
    Model model;
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
  for (Mesh& mesh : cachedModel.GetMeshData()) {
    indexBufSize += (mesh.GetTriangleFaceTable().Size() * TRI_VERTS);
    vertBufSize += mesh.GetVertTable().Size();
  }

  cachedIndexBuffer.Resize(indexBufSize);
  cachedVertBuffer.Resize(vertBufSize, 4, indexBufSize);
}

void World::DebugLoadTriangle(const Vec4& v1, const Vec4& v2, const Vec4& v3)
{
  {
    Model model;
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
  cachedModel = Model(1);
  {
    Mesh& firstMesh = cachedModel[0];
    size_t vertSize = 3 * (sizeof(Vec4) / sizeof(float));
    size_t faceSize = 1;
    firstMesh.Resize(vertSize, 4, faceSize);
    firstMesh.SetData(reinterpret_cast<const float*>(&v1), 0, sizeof(Vec4));
    firstMesh.SetData(reinterpret_cast<const float*>(&v2), 4, sizeof(Vec4));
    firstMesh.SetData(reinterpret_cast<const float*>(&v3), 8, sizeof(Vec4));
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
  cachedVertBuffer.Resize(vertBufSize, 4, indexBufSize);
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
