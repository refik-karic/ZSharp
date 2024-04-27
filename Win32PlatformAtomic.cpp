#if PLATFORM_WINDOWS

#include "PlatformAtomic.h"

#if HW_PLATFORM_X86
#include <intrin.h>
#endif

namespace ZSharp {

PlatformSemaphore::PlatformSemaphore(uint32 maxCount) 
  : mMaxCount(maxCount) {

}

void PlatformSemaphore::Increment() {
  bool aquired = false;
  while (!aquired) {
    long value = _InterlockedIncrement((volatile long*)&mCount);
    if (value > (long)mMaxCount) {
      _InterlockedDecrement((volatile long*)&mCount);
      _mm_pause();
    }
    else {
      aquired = true;
    }
  }
}

int32 PlatformSemaphore::Decrement() {
  return (int32)_InterlockedDecrement((volatile long*)&mCount);
}

void PlatformMutex::Aquire() {
  while (_InterlockedCompareExchange8((volatile char*)&mCount, 1, 0) == 1) {
    _mm_pause();
  }
}

void PlatformMutex::Release() {
  _InterlockedExchange8((volatile char*)&mCount, 0);
}

}

#endif
