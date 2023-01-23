#include "AssetLoader.h"

#include "ZAssert.h"

#include "Array.h"
#include "IndexBuffer.h"
#include "OBJFile.h"
#include "OBJLoader.h"
#include "CommonMath.h"
#include "VertexBuffer.h"

#include <cstring>

namespace ZSharp {
void LoadModelOBJ(const FileString& fileName, Model& model) {
  OBJFile objFile;

  {
    OBJLoader objLoader(objFile, fileName, AssetFormat::Serialized);
  }

  model = Model(1);
  Mesh& mesh = model[0];

  /*
  X, Y, Z, W
  R, G, B
  */
  const size_t stride = 4 + 3; // TODO: Make this based off of the source asset.

  size_t vertSize = objFile.GetVerts().Size() * stride;
  size_t indexSize = objFile.GetFaces().Size();

  mesh.Resize(vertSize, stride, indexSize);

  for (size_t i = 0, triIndex = 0; i < objFile.GetVerts().Size(); ++i) {
    const Vec4& vector = objFile.GetVerts()[i];
    ZAssert(FloatEqual(vector[3], 1.f));

    float vertex[stride];
    memcpy(vertex, reinterpret_cast<const float*>(&vector), sizeof(Vec4));

    const float R[] = { 1.f, 0.f, 0.f };
    const float G[] = { 0.f, 1.f, 0.f };
    const float B[] = { 0.f, 0.f, 1.f };

    switch (triIndex % 3) {
      case 0:
        memcpy(vertex + 4, R, sizeof(R));
        break;
      case 1:
        memcpy(vertex + 4, G, sizeof(G));
        break;
      case 2:
        memcpy(vertex + 4, B, sizeof(B));
        break;
      default:
        break;
    }

    triIndex++;

    mesh.SetData(vertex, i * stride, sizeof(vertex));
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
