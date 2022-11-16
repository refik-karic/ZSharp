#include "AssetLoader.h"

#include "ZAssert.h"

#include "Array.h"
#include "IndexBuffer.h"
#include "OBJFile.h"
#include "CommonMath.h"
#include "VertexBuffer.h"

namespace ZSharp {
void LoadModelOBJ(FileString& fileName, Model& model) {
  OBJFile objFile(fileName, AssetFormat::Serialized);

  model = Model(1);
  Mesh& mesh = model[0];

  // TODO: Need to make sure the faces are in a deterministic order.
  // Blender generates a correct mesh but the faces do not line up properly.
  size_t vertSize = objFile.GetVerts().Size() * (sizeof(Vec4) / sizeof(float));
  size_t indexSize = objFile.GetFaces().Size();
  // TODO: Set the correct stride here.
  mesh.Resize(vertSize, 4, indexSize);

  for (size_t i = 0; i < objFile.GetVerts().Size(); ++i) {
    const Vec4& vector = objFile.GetVerts()[i];
    ZAssert(FloatEqual(vector[3], 1.f));
    mesh.SetData(reinterpret_cast<const float*>(&vector), i * 4, sizeof(Vec4));
  }

  const Array<OBJFace>& faceList = objFile.GetFaces();
  for (size_t triIndex = 0; triIndex < indexSize; ++triIndex) {
    Triangle triangle(static_cast<size_t>(faceList[triIndex].triangleFace[0].vertexIndex),
      static_cast<size_t>(faceList[triIndex].triangleFace[1].vertexIndex),
      static_cast<size_t>(faceList[triIndex].triangleFace[2].vertexIndex)
      );
    mesh.SetTriangle(triangle, triIndex);
  }
}
}
