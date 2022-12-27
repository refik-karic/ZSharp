#ifdef PLATFORM_WINDOWS

#include "PlatformHAL.h"
#include "Win32PlatformHeaders.h"
#include "ZAssert.h"

#include <sysinfoapi.h>

namespace ZSharp {

size_t PlatformGetNumPhysicalCores() {
  ZAssert(false);
  return 0;
}

size_t PlatformGetNumLogicalCores() {
  SYSTEM_INFO info;
  memset(&info, 0, sizeof(info));
  GetSystemInfo(&info);
  return static_cast<size_t>(info.dwNumberOfProcessors);
}

size_t PlatformGetTotalMemory() {
  MEMORYSTATUSEX info;
  memset(&info, 0, sizeof(info));
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
  SYSTEM_INFO info;
  memset(&info, 0, sizeof(info));
  GetSystemInfo(&info);
  return static_cast<size_t>(info.dwPageSize);
}

}

#endif
