#include "AssetLoader.h"

#include <array>
#include <vector>

#include "IndexBuffer.h"
#include "JsonObject.h"
#include "JsonScanner.h"
#include "OBJFile.h"
#include "Token.h"
#include "VertexBuffer.h"

namespace ZSharp {
void LoadModelJSON(const char* fileName, Model& model) {
  JsonScanner scanner;
  JsonObject jsonObject;

  std::vector<Token> tokens;
  scanner.ScanTokens(fileName, tokens);

  if (tokens.empty()) {
    return;
  }

  scanner.PopulateJsonObject(jsonObject, tokens);
  model = Model(jsonObject.GetValue().dataArray.size());

  for (size_t meshIndex = 0; meshIndex < jsonObject.GetValue().dataArray.size(); ++meshIndex) {
    auto indicies = jsonObject.GetValue().dataArray[meshIndex].dataObject.get()->GetValue().dataArray[0].dataObject.get();
    auto verticies = jsonObject.GetValue().dataArray[meshIndex].dataObject.get()->GetValue().dataArray[1].dataObject.get();

    Mesh& mesh = model[meshIndex];
    std::vector<float> vertData(verticies->GetValue().dataArray.size());
    for (size_t i = 0; i < vertData.size(); ++i) {
      vertData[i] = static_cast<float>(verticies->GetValue().dataArray[i].dataFloat);
    }

    mesh.SetData(vertData.data(), vertData.size(), 3, indicies->GetValue().dataArray.size());

    for (size_t triIndex = 0; triIndex < indicies->GetValue().dataArray.size(); ++triIndex) {
      Triangle triangle(static_cast<size_t>(indicies->GetValue().dataArray[triIndex].dataArray[0].dataInt),
        static_cast<size_t>(indicies->GetValue().dataArray[triIndex].dataArray[1].dataInt),
        static_cast<size_t>(indicies->GetValue().dataArray[triIndex].dataArray[2].dataInt));
      mesh.SetTriangle(triangle, triIndex);
    }
  }
}

void LoadModelOBJ(FileString& fileName, Model& model) {
  OBJFile objFile(fileName, AssetFormat::Serialized);

  // TODO: This step needs to be removed, possibly by making a Mesh contain Vec4's rather than Vec3's.
  std::vector<float> scratchBuf;
  scratchBuf.resize(objFile.GetVerts().size() * 3);
  size_t scratchIndex = 0;
  for (const Vec4& vert : objFile.GetVerts()) {
    scratchBuf[scratchIndex++] = vert[0];
    scratchBuf[scratchIndex++] = vert[1];
    scratchBuf[scratchIndex++] = vert[2];
  }

  model = Model(1);
  Mesh& mesh = model[0];

  size_t indexSize = objFile.GetFaces().size();
  // TODO: Set the correct stride here.
  mesh.SetData(scratchBuf.data(), scratchBuf.size() * sizeof(float), 3, indexSize);

  const std::vector<OBJFace>& faceList = objFile.GetFaces();
  for (size_t triIndex = 0; triIndex < indexSize; ++triIndex) {
    Triangle triangle(static_cast<size_t>(faceList[triIndex].triangleFace[0].vertexIndex),
      static_cast<size_t>(faceList[triIndex].triangleFace[1].vertexIndex),
      static_cast<size_t>(faceList[triIndex].triangleFace[2].vertexIndex)
      );
    mesh.SetTriangle(triangle, triIndex);
  }
}
}
