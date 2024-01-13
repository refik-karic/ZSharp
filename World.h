#pragma once

#include "Array.h"
#include "Asset.h"
#include "IndexBuffer.h"
#include "Model.h"
#include "PhysicsObject.h"
#include "ShadingMode.h"
#include "VertexBuffer.h"

namespace ZSharp {
class World final {
  public:

  World();

  void TickPhysics(size_t deltaMs);

  void LoadModels();

  void DebugLoadTriangle(const float* v1, const float* v2, const float* v3, ShadingModeOrder order, int32 stride);

  size_t GetTotalModels() const;

  Array<Model>& GetModels();

  Array<VertexBuffer>& GetVertexBuffers();

  Array<IndexBuffer>& GetIndexBuffers();

  protected:

  private:
  Array<Model> mActiveModels;
  Array<VertexBuffer> mVertexBuffers;
  Array<IndexBuffer> mIndexBuffers;

  Array<PhysicsObject*> mDynamicObjects;
  Array<PhysicsObject*> mStaticObjects;

  void LoadOBJ(Model& model, Asset& asset);
};

}
