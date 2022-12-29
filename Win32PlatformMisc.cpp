#ifdef PLATFORM_WINDOWS

#include "PlatformMisc.h"
#include "Win32PlatformHeaders.h"

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

}

#endif
