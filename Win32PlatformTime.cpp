#ifdef PLATFORM_WINDOWS

#include "PlatformTime.h"

#include "Win32PlatformHeaders.h"
#include "PlatformLogging.h"

namespace ZSharp {

size_t PlatformHighResClock() {
  LARGE_INTEGER ticks;
  if (!QueryPerformanceCounter(&ticks)) {
    PlatformDebugPrintLastError();
    return 0;
  }
  else {
    return ticks.QuadPart;
  }
}

size_t PlatformHighResClockDelta(size_t startingTime, ClockUnits units) {
  LARGE_INTEGER ticks, frequency;

  if (!QueryPerformanceFrequency(&frequency) || !QueryPerformanceCounter(&ticks)) {
    PlatformDebugPrintLastError();
    return 0;
  }
  else {
    ticks.QuadPart -= startingTime;

    size_t scale = 1;

    switch (units) {
      case ClockUnits::Seconds:
        scale = 1;
        break;
      case ClockUnits::Milliseconds:
        scale = 1000;
        break;
      case ClockUnits::Microseconds:
        scale = 1000000;
        break;
    }

    ticks.QuadPart *= scale;
    ticks.QuadPart /= frequency.QuadPart;
    return ticks.QuadPart;
  }
}

}

#endif 
