#include "OBJFile.h"

#include "OBJLoader.h"

namespace ZSharp {
OBJFile::OBJFile() {
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

size_t OBJFile::Stride() const {
  size_t stride = 4;
  for (ShadingMode& mode : mShadingOrder) {
    stride += mode.length;
  }

  return stride;
}

ShadingModeOrder& OBJFile::ShadingOrder() {
  return mShadingOrder;
}

String& OBJFile::AlbedoTexture() {
  return mAlbedoTexture;
}

}
