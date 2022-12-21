#include "Model.h"

#include "Triangle.h"

#define FAST_LOAD 1

namespace ZSharp {
Model::Model() {
}

Model::Model(size_t numMesh) : mData(numMesh) {

}

Model::Model(const Model& copy) {
  *this = copy;
}

size_t Model::MeshCount() const {
  return mData.Size();
}

Array<Mesh>& Model::GetMeshData() {
  return mData;
}

const Array<Mesh>& Model::GetMeshData() const {
  return mData;
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

}
