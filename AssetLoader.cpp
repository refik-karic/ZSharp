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
void LoadModelOBJ(FileString& fileName, Model& model) {
  OBJFile objFile(fileName, AssetFormat::Serialized);

  model = Model(1);
  Mesh& mesh = model[0];

  size_t vertSize = objFile.GetVerts().size() * (sizeof(Vec4) / sizeof(float));
  size_t indexSize = objFile.GetFaces().size();
  // TODO: Set the correct stride here.
  mesh.Resize(vertSize, 4, indexSize);
  mesh.SetData(reinterpret_cast<const float*>(objFile.GetVerts().data()), vertSize);

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
