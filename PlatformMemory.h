#pragma once

#include "ZBaseTypes.h"

// TODO: See if there is a portable way to not have to include this header for placement new.
#include <new>

namespace ZSharp {

void* PlatformMalloc(size_t length);

void* PlatformReAlloc(void* memory, size_t length);

void PlatformFree(void* memory);

void* PlatformAlignedMalloc(size_t length, size_t alignment);

void PlatformAlignedFree(void* alignedMemory);

}
