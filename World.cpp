#include "World.h"

#include "AssetLoader.h"

namespace ZSharp {
World::World() {
}

void World::LoadModel(FileString& path) {
  {
    Model model;
    mActiveModels.push_back(model);
  }

  {
    VertexBuffer vertBuffer;
    mVertexBuffers.push_back(vertBuffer);
  }

  {
    IndexBuffer indexBuffer;
    mIndexBuffers.push_back(indexBuffer);
  }
  
  Model& cachedModel = mActiveModels[mActiveModels.size() - 1];
  LoadModelOBJ(path, cachedModel);

  VertexBuffer& cachedVertBuffer = mVertexBuffers[mVertexBuffers.size() - 1];
  IndexBuffer& cachedIndexBuffer = mIndexBuffers[mIndexBuffers.size() - 1];

  size_t indexBufSize = 0;
  for (Mesh& mesh : cachedModel.GetMeshData()) {
    indexBufSize += (mesh.GetTriangleFaceTable().size() * TRI_VERTS);
  }

  cachedIndexBuffer.Resize(indexBufSize);
  cachedVertBuffer.Resize(indexBufSize * TRI_VERTS, TRI_VERTS);
}

size_t World::GetTotalModels() const {
  return mActiveModels.size();
}

std::vector<Model>& World::GetModels() {
  return mActiveModels;
}

std::vector<VertexBuffer>& World::GetVertexBuffers() {
  return mVertexBuffers;
}

std::vector<IndexBuffer>& World::GetIndexBuffers() {
  return mIndexBuffers;
}
}
