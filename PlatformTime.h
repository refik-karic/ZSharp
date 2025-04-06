#pragma once

#include "ZBaseTypes.h"
#include "ZString.h"

namespace ZSharp {

size_t PlatformHighResClock();

size_t PlatformHighResClockDeltaS(size_t startingTime);

size_t PlatformHighResClockDeltaMs(size_t startingTime);

size_t PlatformHighResClockDeltaUs(size_t startingTime);

String PlatformSystemTimeFormat();

}
