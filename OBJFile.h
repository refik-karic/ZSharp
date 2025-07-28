#pragma once

#include "ZBaseTypes.h"

#include "Array.h"
#include "Vec3.h"
#include "Vec4.h"
#include "ZString.h"
#include "FileString.h"
#include "Span.h"

namespace ZSharp {
struct alignas(8) OBJFaceElement {
  uint32 vertexIndex = 0;
  uint32 uvIndex = 0;
  uint32 normalIndex = 0;
};

struct OBJFace {
  OBJFaceElement triangleFace[3];
};

class OBJFile final {
  public:
  OBJFile();

  void LoadFromFile(const FileString& path);

  Array<Vec4>& Verts();

  Array<Vec3>& Normals();

  Array<Vec3>& UVs();

  Array<OBJFace>& Faces();

  String& AlbedoTexture();

  private:
  Array<Vec4> mVerts;

  Array<Vec3> mNormals;

  Array<Vec3> mUVCoords;

  Array<OBJFace> mFaces;

  String mAlbedoTexture;

  void ParseRaw(const FileString& objFilePath);

  void ParseOBJLine(Span<const char>& line, const FileString& objFilePath);

  void ParseMTLLine(const char* currentLine, size_t length, const FileString& objFilePath);

  void ParseVec3(float fillVec[3], Span<const char>& line, float fallback);

  void ParseVec4(float fillVec[4], Span<const char>& line, float fallback);

  void ParseFace(OBJFace& fillFace, Span<const char>& line);

  void ParseMaterial(Span<const char>& line, const FileString& objFilePath);
};
}
