#pragma once

#include "ZBaseTypes.h"

namespace ZSharp {

size_t PlatformGetNumPhysicalCores();

size_t PlatformGetNumLogicalCores();

size_t PlatformGetTotalMemory();

size_t PlatformGetPageSize();

}
