#ifdef PLATFORM_WINDOWS

#include "PlatformMisc.h"

#include "ZAssert.h"
#include "Logger.h"

#include <Windows.h>
#include <Lmcons.h>

namespace ZSharp {
String PlatformGetBuildType() {
  String buildType(BUILD_TYPE);
  return buildType;
}

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

  monitors.EmplaceBack(String::FromFormat("Display {0}: \n"
    "\tWidth: {1}\n"
    "\tHeight: {2}\n"
    "\tBits per pixel: {3}\n"
    "\tRefresh rate: {4}\n",
    monitors.Size(),
    GetDeviceCaps(hdc, HORZRES),
    GetDeviceCaps(hdc, VERTRES),
    GetDeviceCaps(hdc, BITSPIXEL),
    GetDeviceCaps(hdc, VREFRESH)));

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
