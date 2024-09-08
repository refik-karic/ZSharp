#include "ShaderDefinition.h"

namespace ZSharp {

ShaderDefinition::ShaderDefinition() : mAttributeStride(0), mAttributeLength(0), mShadingMethod(ShadingMethod::None) {}

ShaderDefinition::ShaderDefinition(size_t stride, size_t length, ShadingMethod method) : mAttributeStride(stride), mAttributeLength(length), mShadingMethod(method) {}

ShaderDefinition::ShaderDefinition(const ShaderDefinition& rhs) : mAttributeStride(rhs.mAttributeStride), mAttributeLength(rhs.mAttributeLength), mShadingMethod(rhs.mShadingMethod) {}

ShadingMethod ShaderDefinition::GetShadingMethod() const {
  return mShadingMethod;
}

void ShaderDefinition::SetShadingMethod(ShadingMethod method) {
  mShadingMethod = method;
}

size_t ShaderDefinition::GetAttributeStride() const {
  return mAttributeStride;
}

size_t ShaderDefinition::GetAttributeLength() const {
  return mAttributeLength;
}

void ShaderDefinition::SetAttributeStride(size_t stride) {
  mAttributeStride = stride;
}

void ShaderDefinition::SetAttributeLength(size_t length) {
  mAttributeLength = length;
}

void ShaderDefinition::Serialize(ISerializer& serializer) {
  serializer.Serialize(&mAttributeStride, sizeof(mAttributeStride));
  serializer.Serialize(&mAttributeLength, sizeof(mAttributeLength));
  serializer.Serialize(&mShadingMethod, sizeof(mShadingMethod));
}

void ShaderDefinition::Deserialize(IDeserializer& deserializer) {
  deserializer.Deserialize(&mAttributeStride, sizeof(mAttributeStride));
  deserializer.Deserialize(&mAttributeLength, sizeof(mAttributeLength));
  deserializer.Deserialize(&mShadingMethod, sizeof(mShadingMethod));
}

}
