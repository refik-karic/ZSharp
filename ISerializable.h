#pragma once

#include "ZBaseTypes.h"

namespace ZSharp {

class ISerializer {
  public:

  virtual bool Serialize(const void* memory, size_t sizeBytes) = 0;
};

class IDeserializer {
  public:

  virtual bool Deserialize(void* memory, size_t sizeBytes) = 0;
};

class ISerializable {
  public:
  virtual void Serialize(ISerializer& serializer) = 0;

  virtual void Deserialize(IDeserializer& deserializer) = 0;
};
}
