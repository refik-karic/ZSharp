#pragma once

#include "ZBaseTypes.h"
#include "ZString.h"

namespace ZSharp {

size_t PlatformGetNumPhysicalCores();

size_t PlatformGetNumLogicalCores();

size_t PlatformGetTotalMemory();

size_t PlatformGetPageSize();

}
