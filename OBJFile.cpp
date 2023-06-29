#include "OBJFile.h"

#include "OBJLoader.h"

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

OBJFile::OBJFile(const Asset& asset) {
  if (asset.IsLoose()) {
    OBJLoader loader(asset.LoosePath(), *this);
  }
  else {
    MemoryDeserializer deserializer(asset.Loader());
    OBJLoader loader(deserializer, *this);
  }
}

Array<Vec4>& OBJFile::Verts() {
  return mVerts;
}

Array<Vec3>& OBJFile::Normals() {
  return mNormals;
}

Array<Vec3>& OBJFile::UVs() {
  return mUVCoords;
}

Array<OBJFace>& OBJFile::Faces() {
  return mFaces;
}

size_t& OBJFile::Stride() {
  return mStride;
}

ShadingModeOrder& OBJFile::ShadingOrder() {
  return mShadingOrder;
}

}
