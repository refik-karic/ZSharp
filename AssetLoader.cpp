#include "AssetLoader.h"

#include <array>
#include <cassert>
#include <vector>

#include "IndexBuffer.h"
#include "JsonObject.h"
#include "JsonScanner.h"
#include "OBJFile.h"
#include "Token.h"
#include "UtilMath.h"
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

  for (size_t i = 0; i < objFile.GetVerts().size(); ++i) {
    const Vec4& vector = objFile.GetVerts()[i];
    assert(FloatEqual(vector[3], 1.f));
    mesh.SetData(reinterpret_cast<const float*>(&vector), i * 4, sizeof(Vec4));
  }

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
