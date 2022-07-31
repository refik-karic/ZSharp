#if defined(_WIN64)

#include "PlatformMemory.h"

#include "ZAssert.h"
#include "Win32PlatformHeaders.h"
#include <malloc.h>

void* operator new(size_t size) noexcept(false) {
  void* memory = ZSharp::PlatformMalloc(size);
  ZAssert(memory != nullptr);
  return memory;
}

void* operator new[](size_t size) noexcept(false) {
  void* memory = ZSharp::PlatformMalloc(size);
  ZAssert(memory != nullptr);
  return memory;
}

void operator delete(void* memory) noexcept {
  ZAssert(memory != nullptr);
  ZSharp::PlatformFree(memory);
}

void operator delete[](void* memory) noexcept {
  ZAssert(memory != nullptr);
  ZSharp::PlatformFree(memory);
}

namespace ZSharp {

void* PlatformMalloc(size_t length) {
  HANDLE processHeap = GetProcessHeap();
  return HeapAlloc(processHeap, 0, length);
}

void* PlatformReAlloc(void* memory, size_t length) {
  HANDLE processHeap = GetProcessHeap();
  return HeapReAlloc(processHeap, 0, memory, length);
}

void PlatformFree(void* memory) {
  HANDLE processHeap = GetProcessHeap();
  bool result = HeapFree(processHeap, 0, memory);
  (void)result;
}

void* PlatformAlignedMalloc(size_t length, size_t alignment) {
  return _aligned_malloc(length, alignment);
}

void PlatformAlignedFree(void* alignedMemory) {
  _aligned_free(alignedMemory);
}

}

#endif
