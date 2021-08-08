#pragma once

#include <array>
#include <vector>

#include "JsonObject.h"
#include "Model.h"
#include "JsonScanner.h"
#include "Token.h"

namespace ZSharp {

class AssetLoader final {
  public:
  AssetLoader() = delete;

  template <typename T>
  static void LoadModel(const char* fileName, Model<T>& model) {
    JsonScanner scanner;
    JsonObject jsonObject;

    std::vector<Token> tokens;
    scanner.ScanTokens(fileName, tokens);

    if(tokens.empty()) {
      return;
    }

    scanner.PopulateJsonObject(jsonObject, tokens);
    model = Model<T>(jsonObject.GetValue().dataArray.size());
    
    for (std::size_t meshIndex = 0; meshIndex < jsonObject.GetValue().dataArray.size(); ++meshIndex) {
      auto indicies = jsonObject.GetValue().dataArray[meshIndex].dataObject.get()->GetValue().dataArray[0].dataObject.get();
      auto verticies = jsonObject.GetValue().dataArray[meshIndex].dataObject.get()->GetValue().dataArray[1].dataObject.get();

      Mesh<T>& mesh = model[meshIndex];
      std::vector<T> vertData(verticies->GetValue().dataArray.size());
      for (std::size_t i = 0; i < vertData.size(); ++i) {
        vertData[i] = static_cast<T>(verticies->GetValue().dataArray[i].dataFloat);
      }

      mesh.SetData(vertData.data(), vertData.size(), indicies->GetValue().dataArray.size());

      for (std::size_t triIndex = 0; triIndex < indicies->GetValue().dataArray.size(); ++triIndex) {
        std::array<size_t, 3> triIndicies;
        triIndicies[0] = static_cast<std::size_t>(indicies->GetValue().dataArray[triIndex].dataArray[0].dataInt);
        triIndicies[1] = static_cast<std::size_t>(indicies->GetValue().dataArray[triIndex].dataArray[1].dataInt);
        triIndicies[2] = static_cast<std::size_t>(indicies->GetValue().dataArray[triIndex].dataArray[2].dataInt);
        mesh.SetTriangle(triIndicies, triIndex);
      }
    }
  }

  private:
};
}
