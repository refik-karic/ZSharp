#pragma once

#include "ZBaseTypes.h"
#include "PlatformLogging.h"

namespace ZSharp {

#define NamedScopedTimer(ID) ScopedTimer Timer_##ID(__LINE__, __FILE__, __FUNCTION__)

class ScopedTimer {
  public:

  ScopedTimer(int32 line, const char* file, const char* function);

  ~ScopedTimer();

  private:
  size_t mTime;
  int32 mLine;
  const char* mFile;
  const char* mFunction;
};

}
