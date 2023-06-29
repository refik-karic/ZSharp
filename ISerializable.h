#pragma once

#include "Serializer.h"

namespace ZSharp {

enum class AssetFormat : size_t {
  Raw, // Unchanged source asset, requires parsing.
  Serialized, // Stripped asset that can be batch loaded.
};

class ISerializable {
  public:
  virtual void Serialize(ISerializer& serializer) = 0;

  virtual void Deserialize(IDeserializer& deserializer) = 0;
};
}
