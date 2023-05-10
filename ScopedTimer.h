#pragma once

#include "ZBaseTypes.h"
#include "PlatformDebug.h"

namespace ZSharp {

#define NamedScopedTimer(ID) ScopedTimer Timer_##ID(#ID, __LINE__, __FILE__, __FUNCTION__)

class ScopedTimer {
  public:

  ScopedTimer(const char* name, int32 line, const char* file, const char* function);

  ~ScopedTimer();

  private:
  size_t mTime;
  const char* mName;
  int32 mLine;
  const char* mFile;
  const char* mFunction;
};

}
