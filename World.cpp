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
