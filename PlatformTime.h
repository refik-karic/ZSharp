#pragma once

#include "ZBaseTypes.h"
#include "ZString.h"

namespace ZSharp {

enum class ClockUnits {
  Seconds,
  Milliseconds,
  Microseconds
};

size_t PlatformHighResClock();

size_t PlatformHighResClockDelta(size_t startingTime, ClockUnits units);

String PlatformSystemTimeFormat();

}
