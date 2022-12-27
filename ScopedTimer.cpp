#include "ScopedTimer.h"

#include "Logger.h"
#include "PlatformTime.h"
#include "ZString.h"

#define LOG_SCOPED_TIMERS 1

namespace ZSharp {

ScopedTimer::ScopedTimer(int32 line, const char* file, const char* function)
  : mLine(line), mFile(file), mFunction(function), mTime(PlatformHighResClock()) {

}

ScopedTimer::~ScopedTimer() {
  size_t deltaMicroseconds = PlatformHighResClockDelta(mTime, ClockUnits::Microseconds);

  String log;
  log.Appendf("{0} took {1} us.\n", mFunction, deltaMicroseconds);

#if LOG_SCOPED_TIMERS
  Logger::Log(LogCategory::Perf, log);
#else
  PlatformDebugPrint(log.Str());
#endif
}

}
