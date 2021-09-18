#pragma once

#include "FileString.h"
#include "ISerializable.h"
#include "Vec3.h"
#include "Vec4.h"

namespace ZSharp {
struct alignas(8) OBJFaceElement {
  std::uint64_t vertexIndex = 0;
  std::uint64_t uvIndex = 0;
  std::uint64_t normalIndex = 0;
};

struct OBJFace {
  OBJFaceElement triangleFace[3];
};

enum class AssetFormat : std::size_t {
  Raw, // Unchanged source asset, requires parsing.
  Serialized, // Stripped asset that can be batch loaded.
};

class OBJFile : public ISerializable {
public:
  OBJFile(FileString& objFilePath, AssetFormat format);

  const std::vector<ZSharp::Vec4>& GetVerts();

  const std::vector<OBJFace>& GetFaces();

  virtual void Serialize(FileString& destPath) override;

  virtual void Deserialize(FileString& objFilePath) override;

  void SetVerboseParse(bool state);

protected:

private:
  bool mVerboseParse = false;

  std::vector<ZSharp::Vec4> mVerts;

  std::vector<ZSharp::Vec3> mNormals;

  std::vector<ZSharp::Vec3> mUVCoords;

  std::vector<OBJFace> mFaces;

  void ParseRaw(FileString& objFilePath);

  void ParseLine(std::string& currentLine);

  void ParseVec3(ZSharp::Vec3& fillVec, std::string& line, float fallback);

  void ParseVec4(ZSharp::Vec4& fillVec, std::string& line, float fallback);

  void ParseFace(OBJFace& fillFace, std::string& line);
};
}
