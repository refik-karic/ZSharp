#include "ScopedTimer.h"

#include "PlatformTime.h"
#include "ZString.h"

namespace ZSharp {

ScopedTimer::ScopedTimer(int32 line, const char* file, const char* function)
  : mLine(line), mFile(file), mFunction(function), mTime(PlatformHighResClock()) {

}

ScopedTimer::~ScopedTimer() {
  size_t deltaMicroseconds = PlatformHighResClockDelta(mTime, ClockUnits::Microseconds);

  String log;
  log.Appendf("{0} took {1} us.\n", mFunction, deltaMicroseconds);
  PlatformDebugPrint(log.Str());

  // TODO: Log to stdout or a file perhaps?
}

}
