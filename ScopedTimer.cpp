#include "ScopedTimer.h"

#include "Logger.h"
#include "PlatformTime.h"
#include "ZString.h"

#define LOG_SCOPED_TIMERS 1

namespace ZSharp {

ScopedTimer::ScopedTimer(const char* name)
  : mTime(PlatformHighResClock()), mName(name) {

}

ScopedTimer::~ScopedTimer() {
  size_t deltaMicroseconds = PlatformHighResClockDeltaUs(mTime);

#if LOG_SCOPED_TIMERS
  Logger::Log(LogCategory::Perf, String::FromFormat("{0} took {1} us.\n", mName, deltaMicroseconds));
#else
  PlatformDebugPrint(String::FromFormat("{0} took {1} us.\n", mName, deltaMicroseconds).Str());
#endif
}

}
