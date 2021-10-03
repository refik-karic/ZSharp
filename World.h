#pragma once

#include "FileString.h"
#include "IndexBuffer.h"
#include "Model.h"
#include "VertexBuffer.h"

#include <vector>

namespace ZSharp {
class World final {
  public:

  World();

  void LoadModel(FileString& path);

  size_t GetTotalModels() const;

  std::vector<Model>& GetModels();

  std::vector<VertexBuffer>& GetVertexBuffers();

  std::vector<IndexBuffer>& GetIndexBuffers();

  protected:

  private:
  std::vector<Model> mActiveModels;
  std::vector<VertexBuffer> mVertexBuffers;
  std::vector<IndexBuffer> mIndexBuffers;
};

}
