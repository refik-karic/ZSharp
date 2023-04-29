#pragma once

#include "ZAssert.h"
#include "ZBaseTypes.h"

#include "Array.h"
#include "IndexBuffer.h"
#include "Mat4x4.h"
#include "Mesh.h"
#include "ShadingMode.h"
#include "Vec4.h"
#include "VertexBuffer.h"
#include "WorldObject.h"

namespace ZSharp {

class Model final : public WorldObject {
  public:

  Model() = default;

  Model(const ShadingModeOrder& order, size_t stride);

  Model(const Model& copy);

  void operator=(const Model& rhs) {
    if (this == &rhs) {
      return;
    }

    mShadingOrder = rhs.mShadingOrder;
    mStride = rhs.mStride;
    mData = rhs.mData;
  }

  Mesh& operator[](size_t index) {
    ZAssert(index < mData.Size());
    return mData[index];
  }

  size_t MeshCount() const;

  void CreateNewMesh();

  Array<Mesh>& GetMeshData();

  const Array<Mesh>& GetMeshData() const;

  size_t Stride() const;

  const ShadingModeOrder& ShadingOrder() const;

  void SetShadingOrder(const ShadingModeOrder& order);

  void SetStride(size_t stride);

  void FillBuffers(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer) const;

  private:
  ShadingModeOrder mShadingOrder;
  size_t mStride;
  Array<Mesh> mData;
};

}
