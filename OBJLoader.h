#pragma once

#include "ZBaseTypes.h"
#include "OBJFile.h"
#include "Serializer.h"
#include "ISerializable.h"

namespace ZSharp {

class OBJLoader final : public ISerializable {
  public:
  OBJLoader(OBJFile& file, const FileString& path, AssetFormat format);

  virtual void Serialize(Serializer& serializer) override;

  private:
  OBJFile& mFile;

  virtual void Deserialize(Deserializer& deserializer) override;

  void ParseRaw(const FileString& objFilePath);

  void ParseLine(const char* currentLine);

  void ParseVec3(Vec3& fillVec, String& line, float fallback);

  void ParseVec4(Vec4& fillVec, String& line, float fallback);

  void ParseFace(OBJFace& fillFace, String& line);
};

}
