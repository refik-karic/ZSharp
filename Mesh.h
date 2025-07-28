#pragma once

#include "ZBaseTypes.h"

#include "Array.h"
#include "ISerializable.h"
#include "ShaderDefinition.h"
#include "Triangle.h"
#include "ZString.h"

namespace ZSharp {

class Mesh final : public ISerializable {
  public:

  Mesh() = default;

  Mesh(size_t numVerts, size_t numTriangleFaces);

  Mesh(const Mesh& copy);
  
  void operator=(const Mesh& rhs);

  void Resize(size_t vertexLength, size_t faceTableLength);

  void SetData(const float* vertData, size_t index, size_t numBytes);

  void SetTriangle(const Triangle& triangle, size_t index);

  Array<float>& GetVertTable();

  const Array<float>& GetVertTable() const;

  Array<Triangle>& GetTriangleFaceTable();

  const Array<Triangle>& GetTriangleFaceTable() const;

  const ShaderDefinition& GetShader() const;

  void SetShader(const ShaderDefinition& shader);

  int32& TextureId();

  String& AlbedoTexture();

  size_t Stride() const;

  virtual void Serialize(ISerializer& serializer) override;

  virtual void Deserialize(IDeserializer& deserializer) override;

  private:
  // TODO: Make the bundle generation smarter about texture name to id mapping.
  int32 mTextureId = -1;
  String mAlbedoTexture;
  ShaderDefinition mShader;
  Array<float> mVertTable;
  Array<Triangle> mTriangleFaceTable;
};

}
