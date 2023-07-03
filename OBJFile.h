#pragma once

#include "ZBaseTypes.h"

#include "Asset.h"

#include "Array.h"
#include "Vec3.h"
#include "Vec4.h"
#include "ZString.h"

#include "ShadingMode.h"

namespace ZSharp {
struct alignas(8) OBJFaceElement {
  uint64 vertexIndex = 0;
  uint64 uvIndex = 0;
  uint64 normalIndex = 0;
};

struct OBJFace {
  OBJFaceElement triangleFace[3];
};

class OBJFile final {
  public:
  OBJFile();

  // Load loose or serialized asset from bundle.
  OBJFile(const Asset& asset);

  Array<Vec4>& Verts();

  Array<Vec3>& Normals();

  Array<Vec3>& UVs();

  Array<OBJFace>& Faces();

  size_t Stride() const;

  ShadingModeOrder& ShadingOrder();

  String& AlbedoTexture();

  private:
  Array<Vec4> mVerts;

  Array<Vec3> mNormals;

  Array<Vec3> mUVCoords;

  Array<OBJFace> mFaces;

  String mAlbedoTexture;

  ShadingModeOrder mShadingOrder;
};
}
