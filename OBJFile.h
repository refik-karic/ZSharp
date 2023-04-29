#pragma once

#include "ZBaseTypes.h"

#include "Array.h"
#include "Vec3.h"
#include "Vec4.h"

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

  const Array<Vec4>& GetVerts() const;

  const Array<Vec3>& GetNormals() const;

  const Array<Vec3>& GetUVs() const;

  const Array<OBJFace>& GetFaces() const;

  size_t GetStride() const;

  const ShadingModeOrder& GetShadingOrder() const;

  private:
  Array<Vec4> mVerts;

  Array<Vec3> mNormals;

  Array<Vec3> mUVCoords;

  Array<OBJFace> mFaces;

  size_t mStride = 7; // TODO: Make this based off of the source asset.

  ShadingModeOrder mShadingOrder;

  friend class OBJLoader;
};
}
