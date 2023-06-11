#include "OBJFile.h"

#define DEBUG_TEXTURE 1

namespace ZSharp {
OBJFile::OBJFile() {
#if DEBUG_TEXTURE
  ShadingMode mode(ShadingModes::UV, 2);
  mShadingOrder.PushBack(mode); // TODO: Make this based off of the source asset.
#else
  ShadingMode mode(ShadingModes::RGB, 3);
  mShadingOrder.PushBack(mode); // TODO: Make this based off of the source asset.
#endif
}

const Array<Vec4>& OBJFile::GetVerts() const {
  return mVerts;
}

const Array<Vec3>& OBJFile::GetNormals() const {
  return mNormals;
}

const Array<Vec3>& OBJFile::GetUVs() const {
  return mUVCoords;
}

const Array<OBJFace>& OBJFile::GetFaces() const {
  return mFaces;
}

size_t OBJFile::GetStride() const {
  return mStride;
}

const ShadingModeOrder& OBJFile::GetShadingOrder() const {
  return mShadingOrder;
}

}
