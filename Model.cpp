#include "Model.h"

#include "Triangle.h"

namespace ZSharp {
Model::Model() {
}

Model::Model(std::size_t numMesh) : mData(numMesh) {

}

Model::Model(const Model& copy) {
  *this = copy;
}

std::size_t Model::MeshCount() const {
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
    for (std::size_t i = 0; i < mesh.GetTriangleFaceTable().size(); ++i) {
      const Triangle& triangle = mesh.GetTriangleFaceTable()[i];
      indexBuffer.CopyInputData(triangle.GetData(), i * Constants::TRI_VERTS, Constants::TRI_VERTS);
    }

    vertexBuffer.CopyInputData(mesh.GetVertTable().data(), 0, mesh.GetVertTable().size());
  }
}



}
