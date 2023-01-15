#include "OBJFile.h"

namespace ZSharp {
OBJFile::OBJFile() {
  
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

}
