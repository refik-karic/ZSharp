#pragma once

#include <array>
#include <vector>

#include "FileString.h"
#include "JsonObject.h"
#include "Model.h"
#include "JsonScanner.h"
#include "OBJFile.h"
#include "Token.h"

namespace ZSharp {

class AssetLoader final {
  public:
  AssetLoader() = delete;

  static void LoadModelJSON(const char* fileName, Model& model) {
    JsonScanner scanner;
    JsonObject jsonObject;

    std::vector<Token> tokens;
    scanner.ScanTokens(fileName, tokens);

    if(tokens.empty()) {
      return;
    }

    scanner.PopulateJsonObject(jsonObject, tokens);
    model = Model(jsonObject.GetValue().dataArray.size());
    
    for (std::size_t meshIndex = 0; meshIndex < jsonObject.GetValue().dataArray.size(); ++meshIndex) {
      auto indicies = jsonObject.GetValue().dataArray[meshIndex].dataObject.get()->GetValue().dataArray[0].dataObject.get();
      auto verticies = jsonObject.GetValue().dataArray[meshIndex].dataObject.get()->GetValue().dataArray[1].dataObject.get();

      Mesh& mesh = model[meshIndex];
      std::vector<float> vertData(verticies->GetValue().dataArray.size());
      for (std::size_t i = 0; i < vertData.size(); ++i) {
        vertData[i] = static_cast<float>(verticies->GetValue().dataArray[i].dataFloat);
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

  static void LoadModelOBJ(FileString& fileName, Model& model) {
    OBJFile objFile(fileName, AssetFormat::Serialized);

    // TODO: This step needs to be removed, possibly by making a Mesh contain Vec4's rather than Vec3's.
    std::vector<float> scratchBuf;
    scratchBuf.resize(objFile.GetVerts().size() * 3);
    std::size_t scratchIndex = 0;
    for (const Vec4& vert : objFile.GetVerts()) {
      scratchBuf[scratchIndex++] = vert[0];
      scratchBuf[scratchIndex++] = vert[1];
      scratchBuf[scratchIndex++] = vert[2];
    }

    model = Model(1);
    Mesh& mesh = model[0];

    std::size_t indexSize = objFile.GetFaces().size();
    mesh.SetData(scratchBuf.data(), scratchBuf.size(), indexSize);

    const std::vector<OBJFace>& faceList = objFile.GetFaces();
    for (std::size_t triIndex = 0; triIndex < indexSize; ++triIndex) {
      std::array<size_t, 3> triIndicies;
      triIndicies[0] = static_cast<std::size_t>(faceList[triIndex].triangleFace[0].vertexIndex);
      triIndicies[1] = static_cast<std::size_t>(faceList[triIndex].triangleFace[1].vertexIndex);
      triIndicies[2] = static_cast<std::size_t>(faceList[triIndex].triangleFace[2].vertexIndex);
      mesh.SetTriangle(triIndicies, triIndex);
    }
  }

  private:
};
}
