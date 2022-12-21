#ifdef PLATFORM_WINDOWS

#include "PlatformTime.h"

#include "Win32PlatformHeaders.h"
#include "PlatformDebug.h"

#include <datetimeapi.h>
#include <WinNls.h>

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

String PlatformSystemTimeFormat() {
  const size_t bufferSize = 64;
  char timeString[bufferSize];

  SYSTEMTIME systemTime;
  GetLocalTime(&systemTime);

  String result;

  if (GetTimeFormatA(LOCALE_NAME_USER_DEFAULT,
    0,
    &systemTime,
    NULL,
    timeString,
    bufferSize)) {
    result.Append(timeString);
  }
  else {
    return String("");
  }

  memset(timeString, 0, bufferSize);

  if (GetDateFormatA(LOCALE_NAME_USER_DEFAULT,
    DATE_SHORTDATE,
    &systemTime,
    NULL,
    timeString,
    bufferSize)) {
    result.Append(" ");
    result.Append(timeString);
  }
  else {
    return String("");
  }

  return result;
}

}

#endif 
