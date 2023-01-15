#pragma once

#include "Serializer.h"

namespace ZSharp {

enum class AssetFormat : size_t {
  Raw, // Unchanged source asset, requires parsing.
  Serialized, // Stripped asset that can be batch loaded.
};

class ISerializable {
  public:
  virtual void Serialize(Serializer& serializer) = 0;

  protected:
  virtual void Deserialize(Deserializer& deserializer) = 0;
};
}
