#ifdef PLATFORM_WINDOWS

#include "PlatformTime.h"

#include "Win32PlatformHeaders.h"
#include "PlatformDebug.h"
#include "ZAssert.h"

#include <datetimeapi.h>
#include <WinNls.h>

#define RDTSC_FALLBACK 0

#if !RDTSC_FALLBACK
#include <intrin.h>

#define RDTSC_FREQ_CHECK 0

#if !RDTSC_FREQ_CHECK
#pragma message("Ignoring cpuid check for frequency support. Make sure your processor supports reading base frequency!") 
#endif

#endif

namespace ZSharp {

size_t PlatformHighResClock() {
#if RDTSC_FALLBACK
  LARGE_INTEGER ticks;
  if (!QueryPerformanceCounter(&ticks)) {
    PlatformDebugPrintLastError();
    return 0;
  }
  else {
    return ticks.QuadPart;
  }
#else
  // rdtsc may be executed out of order. Insert cpuid to prevent that.
  int frequencyCheck[4];
  __cpuid(frequencyCheck, 0);

  /*
    Skylake+ uArchs can report back base/max/bus frequency.
    If we're running on an older CPU we have to recompile with a fallback to QPC.
  */

#if RDTSC_FREQ_CHECK
  if (frequencyCheck[0] < 0x16) {
    ZAssert(false);
    return 0;
  }
#endif

  return (size_t)__rdtsc();
#endif
}

size_t PlatformHighResClockDeltaS(size_t startingTime) {
#if RDTSC_FALLBACK
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
#else
  int frequencyCheck[4];
  __cpuid(frequencyCheck, 0x16);

  size_t frequency = frequencyCheck[0];

  size_t ticks = ((size_t)__rdtsc()) - startingTime;
  return ticks / (frequency * 1000000);
#endif
}

size_t PlatformHighResClockDeltaMs(size_t startingTime) {
#if RDTSC_FALLBACK
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
#else
  int frequencyCheck[4];
  __cpuid(frequencyCheck, 0x16);

  size_t frequency = frequencyCheck[0];

  size_t ticks = ((size_t)__rdtsc()) - startingTime;
  return ticks / (frequency * 1000);
#endif
}

size_t PlatformHighResClockDeltaUs(size_t startingTime) {
#if RDTSC_FALLBACK
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
#else
  int frequencyCheck[4];
  __cpuid(frequencyCheck, 0x16);

  size_t frequency = frequencyCheck[0];

  size_t ticks = ((size_t)__rdtsc()) - startingTime;
  return ticks / frequency;
#endif
}

String PlatformSystemTimeFormat() {
  char timeString[64];

  SYSTEMTIME systemTime;
  GetLocalTime(&systemTime);

  String result;

  int32 timeLength = GetTimeFormatA(LOCALE_NAME_USER_DEFAULT,
    0,
    &systemTime,
    NULL,
    timeString,
    sizeof(timeString));

  int32 dateLength = GetDateFormatA(LOCALE_NAME_USER_DEFAULT,
    DATE_SHORTDATE,
    &systemTime,
    NULL,
    timeString + timeLength,
    sizeof(timeString) - timeLength);

  if (timeLength > 0 && dateLength > 0) {
    timeString[timeLength - 1] = ' ';
    result.Append(timeString, 0, timeLength + dateLength - 1);
  }

  return result;
}

}

#endif 
