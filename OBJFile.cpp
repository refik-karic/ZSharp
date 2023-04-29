#include "OBJFile.h"

namespace ZSharp {
OBJFile::OBJFile() {
  ShadingMode mode(ShadingModes::RGB, 3);
  mShadingOrder.PushBack(mode); // TODO: Make this based off of the source asset.
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
