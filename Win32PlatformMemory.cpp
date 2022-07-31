#if defined(_WIN64)

#include "PlatformMemory.h"

#include <malloc.h>

namespace ZSharp {

void* PlatformAlignedMalloc(size_t length, size_t alignment) {
  return _aligned_malloc(length, alignment);
}

void PlatformAlignedFree(void* alignedMemory) {
  _aligned_free(alignedMemory);
}

}

#endif
