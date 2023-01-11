#pragma once

#include "ZBaseTypes.h"

#include "Array.h"
#include "FileString.h"
#include "ISerializable.h"
#include "ZString.h"
#include "Vec3.h"
#include "Vec4.h"

namespace ZSharp {
struct alignas(8) OBJFaceElement {
  uint64 vertexIndex = 0;
  uint64 uvIndex = 0;
  uint64 normalIndex = 0;
};

struct OBJFace {
  OBJFaceElement triangleFace[3];
};

enum class AssetFormat : size_t {
  Raw, // Unchanged source asset, requires parsing.
  Serialized, // Stripped asset that can be batch loaded.
};

class OBJFile : public ISerializable {
public:
  OBJFile(FileString& objFilePath, AssetFormat format);

  const Array<Vec4>& GetVerts();

  const Array<OBJFace>& GetFaces();

  virtual void Serialize(FileString& destPath) override;

  virtual void Deserialize(FileString& objFilePath) override;

protected:

private:
  Array<Vec4> mVerts;

  Array<Vec3> mNormals;

  Array<Vec3> mUVCoords;

  Array<OBJFace> mFaces;

  void ParseRaw(FileString& objFilePath);

  void ParseLine(const char* currentLine);

  void ParseVec3(Vec3& fillVec, String& line, float fallback);

  void ParseVec4(Vec4& fillVec, String& line, float fallback);

  void ParseFace(OBJFace& fillFace, String& line);
};
}
