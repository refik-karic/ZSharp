#pragma once

#include "ZBaseTypes.h"

#include "Asset.h"

#include "Array.h"
#include "Vec3.h"
#include "Vec4.h"

#include "ShadingMode.h"

#define DEBUG_TEXTURE_OBJ 1

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

  size_t& Stride();

  ShadingModeOrder& ShadingOrder();

  private:
  Array<Vec4> mVerts;

  Array<Vec3> mNormals;

  Array<Vec3> mUVCoords;

  Array<OBJFace> mFaces;

#if DEBUG_TEXTURE_OBJ
  size_t mStride = 6; // TODO: Make this based off of the source asset.
#else
  size_t mStride = 7; // TODO: Make this based off of the source asset.
#endif

  ShadingModeOrder mShadingOrder;
};
}
