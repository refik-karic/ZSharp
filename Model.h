#pragma once

#include "ZAssert.h"
#include "ZBaseTypes.h"

#include "Array.h"
#include "IndexBuffer.h"
#include "Mat4x4.h"
#include "Mesh.h"
#include "Vec4.h"
#include "VertexBuffer.h"
#include "WorldObject.h"

namespace ZSharp {

class Model final : public WorldObject {
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
    ZAssert(index < mData.Size());
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
