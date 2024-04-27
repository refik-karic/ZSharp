#pragma once

#include "ZBaseTypes.h"

namespace ZSharp {

class PlatformSemaphore final {
  public:

  PlatformSemaphore() = delete;

  PlatformSemaphore(uint32 maxCount);

  PlatformSemaphore(const PlatformSemaphore&) = delete;
  void operator=(const PlatformSemaphore&) = delete;

  void Increment();

  int32 Decrement();

  private:
  uint32 mMaxCount = 0;
  volatile int32 mCount = 0;
};

class PlatformMutex final {
  public:

  PlatformMutex() = default;

  PlatformMutex(const PlatformMutex&) = delete;
  void operator=(const PlatformMutex&) = delete;

  void Aquire();

  void Release();

  private:
  volatile uint8 mCount = 0;
};

}
