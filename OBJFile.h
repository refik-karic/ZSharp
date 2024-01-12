#pragma once

#include "ZBaseTypes.h"

#include "Asset.h"

#include "Array.h"
#include "Vec3.h"
#include "Vec4.h"
#include "ZString.h"
#include "Serializer.h"
#include "FileString.h"
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

  void LoadFromFile(const FileString& path);

  void Serialize(MemorySerializer& serializer);

  void Deserialize(MemoryDeserializer& deserializer);

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

  void ParseRaw(const FileString& objFilePath);

  void ParseOBJLine(const char* currentLine, size_t length, const FileString& objFilePath);

  void ParseMTLLine(const char* currentLine, size_t length, const FileString& objFilePath);

  void ParseVec3(float fillVec[3], String& line, float fallback);

  void ParseVec4(float fillVec[4], String& line, float fallback);

  void ParseFace(OBJFace& fillFace, String& line);

  void ParseMaterial(String& line, const FileString& objFilePath);
};
}
