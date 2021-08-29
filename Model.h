#pragma once

#include <cstddef>
#include <vector>

#include "IndexBuffer.h"
#include "Mesh.h"
#include "Triangle.h"
#include "VertexBuffer.h"

namespace ZSharp {

class Model final {
  public:

  Model() {}

  Model(std::size_t numMesh) : mData(numMesh) {
    
  }

  Model(const Model& copy) {
    *this = copy;
  }

  void operator=(const Model& rhs) {
    if (this == &rhs) {
      return;
    }

    mData = rhs.mData;
  }

  Mesh& operator[](std::size_t index) {
    return mData[index];
  }

  std::size_t MeshCount() const {
    return mData.size();
  }

  std::vector<Mesh>& GetMeshData() {
    return mData;
  }

  const std::vector<Mesh>& GetMeshData() const {
    return mData;
  }

  void FillBuffers(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer) const {
    for(const Mesh& mesh : mData) {
      for(std::size_t i = 0; i < mesh.GetTriangleFaceTable().size(); ++i) {
        const Triangle& triangle = mesh.GetTriangleFaceTable()[i];
        indexBuffer.CopyInputData(triangle.GetData(), i * Constants::TRI_VERTS, Constants::TRI_VERTS);
      }

      vertexBuffer.CopyInputData(mesh.GetVertTable().data(), 0, mesh.GetVertTable().size());
    }
  }

  private:
  std::vector<Mesh> mData;
};

}
