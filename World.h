#pragma once

#include "Array.h"
#include "Asset.h"
#include "FileString.h"
#include "IndexBuffer.h"
#include "Model.h"
#include "ShadingMode.h"
#include "VertexBuffer.h"

namespace ZSharp {
class World final {
  public:

  World();

  void TickPhysics();

  void LoadModels();

  void DebugLoadTriangle(const float* v1, const float* v2, const float* v3, ShadingModeOrder order, size_t stride);

  size_t GetTotalModels() const;

  Array<Model>& GetModels();

  Array<VertexBuffer>& GetVertexBuffers();

  Array<IndexBuffer>& GetIndexBuffers();

  protected:

  private:
  Array<Model> mActiveModels;
  Array<VertexBuffer> mVertexBuffers;
  Array<IndexBuffer> mIndexBuffers;

  void LoadOBJ(Model& model, Asset& asset);
};

}
