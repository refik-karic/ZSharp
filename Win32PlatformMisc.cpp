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

}

#endif
