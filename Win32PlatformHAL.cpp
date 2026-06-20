#ifdef PLATFORM_WINDOWS

#include "PlatformHAL.h"
#include "PlatformMemory.h"
#include "Win32PlatformHeaders.h"
#include "ZAssert.h"
#include "ZBaseTypes.h"

#include <sysinfoapi.h>

namespace ZSharp {

size_t PlatformGetNumPhysicalCores() {
  static size_t numProcessors = 0;

  // Only check once, this Win32 API requires malloc/free.
  // The count of processors will never change during a programs execution.
  if (numProcessors > 0) {
    return numProcessors;
  }

  DWORD structSize = sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
  DWORD size = structSize;
  PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)PlatformCalloc(size);

  bool ret = GetLogicalProcessorInformation(ptr, &size);

  if (!ret && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)) {
    ptr = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)PlatformReAlloc(ptr, size);
    ret = GetLogicalProcessorInformation(ptr, &size);
  }
  
  if (ret) {
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION currentPtr = ptr;
    size_t totalProcessors = 0;

    for (size_t offset = 0; offset + structSize < size; offset += structSize) {
      switch (currentPtr->Relationship) {
        case RelationProcessorCore:
          ++totalProcessors;
          break;
        default:
          break;
      }

      ++currentPtr;
    }

    PlatformFree(ptr);

    numProcessors = totalProcessors;
    return numProcessors;
  }
  else {
    PlatformFree(ptr);
    ZAssert(false);
    return 0;
  }
}

size_t PlatformGetNumLogicalCores() {
  SYSTEM_INFO info{};
  GetSystemInfo(&info);
  return static_cast<size_t>(info.dwNumberOfProcessors);
}

size_t PlatformGetTotalMemory() {
  MEMORYSTATUSEX info{};
  info.dwLength = sizeof(info);

  bool ret = GlobalMemoryStatusEx(&info);

  if (ret) {
    return static_cast<size_t>(info.ullTotalPhys);
  }
  else {
    return 0;
  }
}

size_t PlatformGetPageSize() {
  SYSTEM_INFO info{};
  GetSystemInfo(&info);
  return static_cast<size_t>(info.dwPageSize);
}

size_t PlatformGetL1DCacheSize() {
  static size_t l1Size = 0;

  // Only check once, this Win32 API requires malloc/free.
  // The count of processors will never change during a programs execution.
  if (l1Size > 0) {
    return l1Size;
  }

  DWORD structSize = sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
  DWORD size = structSize;
  PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)PlatformCalloc(size);

  bool ret = GetLogicalProcessorInformation(ptr, &size);

  if (!ret && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)) {
    ptr = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)PlatformReAlloc(ptr, size);
    ret = GetLogicalProcessorInformation(ptr, &size);
  }

  if (ret) {
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION currentPtr = ptr;

    for (size_t offset = 0; offset + structSize < size; offset += structSize) {
      switch (currentPtr->Relationship) {
        case RelationCache:
        {
          if ((currentPtr->Cache.Level == 1) && (currentPtr->Cache.Type == CacheData)) {
            l1Size = currentPtr->Cache.Size;
          }
        }
          break;
        default:
          break;
      }

      if (l1Size > 0) {
        break;
      }

      ++currentPtr;
    }

    PlatformFree(ptr);
    return l1Size;
  }
  else {
    PlatformFree(ptr);
    ZAssert(false);
    return 0;
  }
}

size_t PlatformGetL2DCacheSize() {
  static size_t l2Size = 0;

  // Only check once, this Win32 API requires malloc/free.
  // The count of processors will never change during a programs execution.
  if (l2Size > 0) {
    return l2Size;
  }

  DWORD structSize = sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
  DWORD size = structSize;
  PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)PlatformCalloc(size);

  bool ret = GetLogicalProcessorInformation(ptr, &size);

  if (!ret && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)) {
    ptr = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)PlatformReAlloc(ptr, size);
    ret = GetLogicalProcessorInformation(ptr, &size);
  }

  if (ret) {
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION currentPtr = ptr;

    for (size_t offset = 0; offset + structSize < size; offset += structSize) {
      switch (currentPtr->Relationship) {
      case RelationCache:
      {
        if ((currentPtr->Cache.Level == 2) && (currentPtr->Cache.Type != CacheInstruction)) {
          l2Size = currentPtr->Cache.Size;
        }
      }
      break;
      default:
        break;
      }

      if (l2Size > 0) {
        break;
      }

      ++currentPtr;
    }

    PlatformFree(ptr);
    return l2Size;
  }
  else {
    PlatformFree(ptr);
    ZAssert(false);
    return 0;
  }
}

size_t PlatformGetL3DCacheSize() {
  static size_t l3Size = 0;

  // Only check once, this Win32 API requires malloc/free.
  // The count of processors will never change during a programs execution.
  if (l3Size > 0) {
    return l3Size;
  }

  DWORD structSize = sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
  DWORD size = structSize;
  PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)PlatformCalloc(size);

  bool ret = GetLogicalProcessorInformation(ptr, &size);

  if (!ret && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)) {
    ptr = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)PlatformReAlloc(ptr, size);
    ret = GetLogicalProcessorInformation(ptr, &size);
  }

  if (ret) {
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION currentPtr = ptr;

    for (size_t offset = 0; offset + structSize < size; offset += structSize) {
      switch (currentPtr->Relationship) {
      case RelationCache:
      {
        if ((currentPtr->Cache.Level == 3) && (currentPtr->Cache.Type != CacheInstruction)) {
          l3Size = currentPtr->Cache.Size;
        }
      }
      break;
      default:
        break;
      }

      if (l3Size > 0) {
        break;
      }

      ++currentPtr;
    }

    PlatformFree(ptr);
    return l3Size;
  }
  else {
    PlatformFree(ptr);
    ZAssert(false);
    return 0;
  }
}

}

#endif
