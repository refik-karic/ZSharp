#pragma once

#include "Array.h"
#include "FileString.h"
#include "IndexBuffer.h"
#include "Model.h"
#include "VertexBuffer.h"

namespace ZSharp {
class World final {
  public:

  World();

  void LoadModel(FileString& path);

  size_t GetTotalModels() const;

  Array<Model>& GetModels();

  Array<VertexBuffer>& GetVertexBuffers();

  Array<IndexBuffer>& GetIndexBuffers();

  protected:

  private:
  Array<Model> mActiveModels;
  Array<VertexBuffer> mVertexBuffers;
  Array<IndexBuffer> mIndexBuffers;
};

}
