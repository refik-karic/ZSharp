#pragma once

#include "ZAssert.h"
#include "ZBaseTypes.h"

#include "Array.h"
#include "IndexBuffer.h"
#include "Mat4x4.h"
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
    ZAssert(index < mData.Size());
    return mData[index];
  }

  size_t MeshCount() const;

  Array<Mesh>& GetMeshData();

  const Array<Mesh>& GetMeshData() const;

  void FillBuffers(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer) const;

  void SetRotation(const Mat4x4& rotation);

  const Mat4x4& GetRotation() const;

  private:
  Array<Mesh> mData;

  Vec4 mScale;
  Mat4x4 mRotation;
  Vec4 mTranslation;
};

}
