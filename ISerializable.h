#pragma once

#include "FileString.h"

namespace ZSharp {
class ISerializable {
public:
  virtual void Serialize(FileString& destPath) = 0;

  virtual void Deserialize(FileString& srcPath) = 0;
};
}
