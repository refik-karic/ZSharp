#pragma once

#include "ZBaseTypes.h"
#include "PlatformDebug.h"

namespace ZSharp {

#define NamedScopedTimer(ID) ScopedTimer Timer_##ID(#ID)

class ScopedTimer {
  public:

  ScopedTimer(const char* name);

  ~ScopedTimer();

  private:
  size_t mTime;
  const char* mName;
};

}
