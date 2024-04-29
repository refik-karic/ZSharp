#ifdef PLATFORM_WINDOWS

#include "PlatformThread.h"

#include "Array.h"
#include "PlatformHAL.h"

#include "Win32PlatformHeaders.h"
#include <processthreadsapi.h>
#include <synchapi.h>
#include <winnt.h>
#include <handleapi.h>

#if HW_PLATFORM_X86
#include <intrin.h>
#include <emmintrin.h>
#endif

namespace ZSharp {

struct PlatformThread {
  HANDLE threadHandle;
  DWORD threadId;
};

struct PlatformMonitor {
  HANDLE monitorHandle;
};

PlatformThread* PlatformCreateThread(PlatformThreadFunction threadFunction, void* threadData) {
  HANDLE handle;
  DWORD id;

  handle = CreateThread(
    NULL,
    0,
    (DWORD (*)(void*))threadFunction,
    threadData,
    0,
    &id
  );

  if (handle == NULL) {
    return nullptr;
  }

  PlatformThread* thread = new PlatformThread;
  thread->threadHandle = handle;
  thread->threadId = id;

  return thread;
}

#pragma warning (disable : 4334)

void PlatformPinThreadsToProcessors(PlatformThread** threads, size_t numThreads, bool logical) {
  if (threads == nullptr) {
    return;
  }

  if (logical) {
    if (numThreads > PlatformGetNumLogicalCores()) {
      return;
    }

    PlatformThread** currentThread = threads;
    for (size_t i = 0; i < numThreads; ++i) {
      SetThreadAffinityMask((*currentThread)->threadHandle, 1U << i);
      ++currentThread;
    }
  }
  else {
    if (numThreads > PlatformGetNumPhysicalCores()) {
      return;
    }

    PlatformThread** currentThread = threads;
    for (size_t i = 0; i < numThreads; ++i) {
      SetThreadAffinityMask((*currentThread)->threadHandle, 1U << (i * 2));
      ++currentThread;
    }
  }
}

#pragma warning (default : 4334)

void PlatformJoinThread(PlatformThread* thread) {
  if (thread == nullptr) {
    return;
  }

  WaitForSingleObject(thread->threadHandle, INFINITE);
  CloseHandle(thread->threadHandle);

  delete thread;
}

void PlatformJoinThreadPool(PlatformThread** threads, size_t numThreads) {
  if (threads == nullptr) {
    return;
  }

  Array<HANDLE> handles;
  PlatformThread** currentThread = threads;
  for (size_t i = 0; i < numThreads; ++i) {
    handles.PushBack((*currentThread)->threadHandle);
    ++currentThread;
  }

  WaitForMultipleObjects((DWORD)numThreads, handles.GetData(), true, INFINITE);

  for (HANDLE handle : handles) {
    CloseHandle(handle);
  }

  currentThread = threads;
  for (size_t i = 0; i < numThreads; ++i) {
    delete (*currentThread);
    ++currentThread;
  }
}

PlatformMonitor* PlatformCreateMonitor() {
  HANDLE handle = CreateEventA(NULL, true, false, NULL);

  if (handle == NULL) {
    return nullptr;
  }

  PlatformMonitor* monitor = new PlatformMonitor;
  monitor->monitorHandle = handle;
  return monitor;
}

void PlatformWaitMonitor(PlatformMonitor* monitor) {
  if (monitor == nullptr) {
    return;
  }

  WaitForSingleObject(monitor->monitorHandle, INFINITE);
}

void PlatformSignalMonitor(PlatformMonitor* monitor) {
  if (monitor != nullptr) {
    SetEvent(monitor->monitorHandle);
  }
}

void PlatformClearMonitor(PlatformMonitor* monitor) {
  if (monitor != nullptr) {
    ResetEvent(monitor->monitorHandle);
  }
}

void PlatformDestroyMonitor(PlatformMonitor* monitor) {
  if (monitor == nullptr) {
    return;
  }

  CloseHandle(monitor->monitorHandle);
}

void PlatformYieldThread() {
  Sleep(0);
}

void PlatformBusySpin() {
#if HW_PLATFORM_X86

  int buffer[4] = {};
  __cpuid(buffer, 0x07);

  // Check for tpause support.
  if (((buffer[2] >> 5) & 1U) == 1) {
    // Get the base frequency.
    __cpuid(buffer, 0x16);

    // Wait 1us (i.e. on a 3.2GHz base, we wait 3200 cycles)
    // The 1us value is conventiently provided via the base frequency in MHz.
    DWORD64 waitTime = __rdtsc() + buffer[0];
    // 0 = slow wakeup
    // 1 = fast wakeup
    _tpause(0x01, waitTime);
  }
  else {
    _mm_pause();
  }
#endif
}

}

#endif
