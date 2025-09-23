#pragma once

#include "ZBaseTypes.h"

// Required for placement new.
#include <new>

namespace ZSharp {

void* PlatformMalloc(size_t length);

void* PlatformCalloc(size_t length);

void* PlatformReAlloc(void* memory, size_t length);

void PlatformFree(void* memory);

void* PlatformAlignedMalloc(size_t length, size_t alignment);

void* PlatformAlignedCalloc(size_t length, size_t alignment);

void* PlatformAlignedReAlloc(void* memory, size_t length, size_t alignment);

void PlatformAlignedFree(void* alignedMemory);

}
