﻿#pragma once

#include "ZBaseTypes.h"

#include "AABB.h"
#include "Array.h"
#include "IndexBuffer.h"
#include "Mesh.h"
#include "ShadingMode.h"
#include "Vec4.h"
#include "VertexBuffer.h"
#include "PhysicsObject.h"

namespace ZSharp {

class Model final : public PhysicsObject {
  public:

  Model() = default;

  Model(const ShadingModeOrder& order, size_t stride);

  Model(const Model& copy);

  void operator=(const Model& rhs);

  Mesh& operator[](size_t index);

  size_t MeshCount() const;

  void CreateNewMesh();

  Array<Mesh>& GetMeshData();

  const Array<Mesh>& GetMeshData() const;

  size_t Stride() const;

  const ShadingModeOrder& ShadingOrder() const;

  void SetShadingOrder(const ShadingModeOrder& order);

  void SetStride(size_t stride);

  void FillBuffers(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer) const;

  int32& TextureId();

  AABB ComputeBoundingBox() const;

  private:
  ShadingModeOrder mShadingOrder;
  size_t mStride;
  Array<Mesh> mData;
  int32 mTextureId = -1;
};

}
