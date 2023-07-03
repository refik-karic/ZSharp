#pragma once

#include "ZBaseTypes.h"
#include "OBJFile.h"
#include "Serializer.h"
#include "ISerializable.h"

namespace ZSharp {

class OBJLoader final : public ISerializable {
  public:

  // Load OBJ file from disk. Can be used both for loose loading or during packaging.
  OBJLoader(const FileString& path, OBJFile& objFile);

  // Load serialized OBJ file from a packaged bundle.
  OBJLoader(IDeserializer& deserializer, OBJFile& objFile);

  virtual void Serialize(ISerializer& serializer) override;

  virtual void Deserialize(IDeserializer& deserializer) override;

  private:
  OBJFile& mObjFile;

  void ParseRaw(const FileString& objFilePath);

  void ParseLine(const char* currentLine, size_t length);

  void ParseVec3(Vec3& fillVec, String& line, float fallback);

  void ParseVec4(Vec4& fillVec, String& line, float fallback);

  void ParseFace(OBJFace& fillFace, String& line);
};

}
