#ifdef PLATFORM_WINDOWS

#include "PlatformMisc.h"

#include "ZAssert.h"
#include "Logger.h"

#include <Windows.h>
#include <Lmcons.h>

namespace ZSharp {

String PlatformGetUsername() {
  char buffer[UNLEN + 1];
  memset(buffer, 0, sizeof(buffer));
  DWORD size = sizeof(buffer);
  if (GetUserNameA(buffer, &size)) {
    const String username(buffer);
    return username;
  }
  else {
    return String("");
  }
}

String PlatformGetMachineName() {
  char buffer[MAX_COMPUTERNAME_LENGTH + 1];
  memset(buffer, 0, sizeof(buffer));
  DWORD size = sizeof(buffer);

  if (GetComputerNameA(buffer, &size)) {
    const String name(buffer);
    return name;
  }
  else {
    return String("");
  }
}

BOOL MonitorCallback(
  HMONITOR unnamedParam1,
  HDC hdc,
  LPRECT unnamedParam3,
  LPARAM addressParam) {
  (void)unnamedParam1;
  (void)unnamedParam3;
  Array<String>& monitors = *reinterpret_cast<Array<String>*>(addressParam);

  String monitorInfo(String::FromFormat("Display {0}: \n", monitors.Size()));
  monitorInfo.Appendf("\tWidth: {0}\n", GetDeviceCaps(hdc, HORZRES));
  monitorInfo.Appendf("\tHeight: {0}\n", GetDeviceCaps(hdc, VERTRES));
  monitorInfo.Appendf("\tBits per pixel: {0}\n", GetDeviceCaps(hdc, BITSPIXEL));
  monitorInfo.Appendf("\tRefresh rate: {0}\n", GetDeviceCaps(hdc, VREFRESH));

  monitors.PushBack(monitorInfo);

  return true;
}

Array<String> PlatformEnumDisplayInfo() {
  HDC hdc = GetDC(nullptr);
  ZAssert(hdc != nullptr);

  Array<String> monitors;
  EnumDisplayMonitors(hdc, NULL, &MonitorCallback, (LPARAM)&monitors);
  return monitors;
}

}

#endif
