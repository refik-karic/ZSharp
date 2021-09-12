#pragma once

#include <cstddef>
#include <vector>

#include "IndexBuffer.h"
#include "Mesh.h"
#include "VertexBuffer.h"

namespace ZSharp {

class Model final {
  public:

  Model();

  Model(std::size_t numMesh);

  Model(const Model& copy);

  void operator=(const Model& rhs) {
    if (this == &rhs) {
      return;
    }

    mData = rhs.mData;
  }

  Mesh& operator[](std::size_t index) {
    return mData[index];
  }

  std::size_t MeshCount() const;

  std::vector<Mesh>& GetMeshData();

  const std::vector<Mesh>& GetMeshData() const;

  void FillBuffers(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer) const;

  private:
  std::vector<Mesh> mData;
};

}
