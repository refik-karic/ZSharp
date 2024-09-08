#pragma once

#include "ZBaseTypes.h"

#include "ISerializable.h"

namespace ZSharp {

/*
  Add more shading functions as needed. This is currently a barebones list for testing.
  i.e. ShadingMethod::BlinnPhong may need UV, Specular, etc
*/
enum class ShadingMethod {
  None,
  RGB,
  UV,
  Normals
};

class ShaderDefinition final : public ISerializable {
  public:

  ShaderDefinition();
  ShaderDefinition(size_t stride, size_t length, ShadingMethod method);
  ShaderDefinition(const ShaderDefinition& rhs);

  ShadingMethod GetShadingMethod() const;

  void SetShadingMethod(ShadingMethod method);

  size_t GetAttributeStride() const;

  size_t GetAttributeLength() const;

  void SetAttributeStride(size_t stride);

  void SetAttributeLength(size_t length);

  virtual void Serialize(ISerializer& serializer) override;

  virtual void Deserialize(IDeserializer& deserializer) override;

  private:
  size_t mAttributeStride;
  size_t mAttributeLength;
  ShadingMethod mShadingMethod;
};

}

