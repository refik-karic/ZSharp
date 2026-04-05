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

bool PlatformSetThreadName(PlatformThread* thread, const String& name) {
  if (!thread) {
    return false;
  }

  typedef LONG(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

  HMODULE module = GetModuleHandleA("ntdll.dll");
  if (!module) {
    return false;
  }

  RtlGetVersionPtr rtlGetVersonFunc = (RtlGetVersionPtr)GetProcAddress(module, "RtlGetVersion");
  if (!rtlGetVersonFunc) {
    return false;
  }

  RTL_OSVERSIONINFOW VersionInfo{};
  VersionInfo.dwOSVersionInfoSize = sizeof(VersionInfo);
  rtlGetVersonFunc(&VersionInfo);

  // Extract values
  DWORD Major = VersionInfo.dwMajorVersion;
  DWORD Minor = VersionInfo.dwMinorVersion;
  DWORD Build = VersionInfo.dwBuildNumber;

  // Requires at least Win 10 1607
  if (Major >= 10 && Minor >= 0 && Build >= 14393) {
    return SetThreadDescription(thread->threadHandle, name.ToWide().Str()) == S_OK;
  }
  else {
    return false;
  }
}

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

  Array<HANDLE> handles(numThreads);
  PlatformThread** currentThread = threads;
  for (size_t i = 0; i < numThreads; ++i) {
    handles[i] = (currentThread[i])->threadHandle;
  }

  WaitForMultipleObjects((DWORD)numThreads, handles.GetData(), true, INFINITE);

  for (HANDLE handle : handles) {
    CloseHandle(handle);
  }

  for (size_t i = 0; i < numThreads; ++i) {
    delete currentThread[i];
  }
}

PlatformMonitor* PlatformCreateMonitor(bool signaled) {
  HANDLE handle = CreateEventA(NULL, true, signaled, NULL);

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

void PlatformWaitMonitors(PlatformMonitor** monitors, size_t count) {
  if (monitors == nullptr) {
    return;
  }

  Array<HANDLE> handles(count);
  PlatformMonitor** currentMonitor = monitors;
  for (size_t i = 0; i < count; ++i) {
    handles[i] = (currentMonitor[i])->monitorHandle;
  }

  WaitForMultipleObjects((DWORD)count, handles.GetData(), true, INFINITE);
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

#if 0
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
#else
  _mm_pause();
#endif
#endif
}

}

#endif
