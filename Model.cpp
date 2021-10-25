#include "Model.h"

#include "Triangle.h"

namespace ZSharp {
Model::Model() {
}

Model::Model(size_t numMesh) : mData(numMesh) {

}

Model::Model(const Model& copy) {
  *this = copy;
}

size_t Model::MeshCount() const {
  return mData.size();
}

std::vector<Mesh>& Model::GetMeshData() {
  return mData;
}

const std::vector<Mesh>& Model::GetMeshData() const {
  return mData;
}

void Model::FillBuffers(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer) const {
  for (const Mesh& mesh : mData) {
    indexBuffer.CopyInputData(reinterpret_cast<const size_t*>(mesh.GetTriangleFaceTable().data()), 0, mesh.GetTriangleFaceTable().size() * TRI_VERTS);
    vertexBuffer.CopyInputData(mesh.GetVertTable().data(), 0, mesh.GetVertTable().size());
  }
}

}
