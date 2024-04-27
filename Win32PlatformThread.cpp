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
#include <emmintrin.h>
#endif

namespace ZSharp {

struct PlatformThread {
  HANDLE threadHandle;
  DWORD threadId;
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

void PlatformYieldThread() {
  Sleep(0);
}

void PlatformBusySpin() {
#if HW_PLATFORM_X86
  _mm_pause();
#endif
}

}

#endif
