#pragma once

#include "ZBaseTypes.h"

namespace ZSharp {

void* PlatformAlignedMalloc(size_t length, size_t alignment);

void PlatformAlignedFree(void* alignedMemory);

}
