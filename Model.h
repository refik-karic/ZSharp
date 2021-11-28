﻿#pragma once

#include <cstddef>

#include "Array.h"
#include "IndexBuffer.h"
#include "Mesh.h"
#include "Vec4.h"
#include "VertexBuffer.h"

namespace ZSharp {

class Model final {
  public:

  Model();

  Model(size_t numMesh);

  Model(const Model& copy);

  void operator=(const Model& rhs) {
    if (this == &rhs) {
      return;
    }

    mData = rhs.mData;
  }

  Mesh& operator[](size_t index) {
    return mData[index];
  }

  size_t MeshCount() const;

  Array<Mesh>& GetMeshData();

  const Array<Mesh>& GetMeshData() const;

  void FillBuffers(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer) const;

  private:
  Array<Mesh> mData;
};

}
