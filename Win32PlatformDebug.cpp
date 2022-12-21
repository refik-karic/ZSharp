#ifdef PLATFORM_WINDOWS

#include "PlatformDebug.h"

#include "Win32PlatformHeaders.h"

#include <debugapi.h>
#include <errhandlingapi.h>
#include <WinBase.h>

namespace ZSharp {

void PlatformDebugPrintLastError() {
#ifndef NDEBUG
  DWORD lastError = GetLastError();
  LPCTSTR errorString = NULL;
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS |
    FORMAT_MESSAGE_ARGUMENT_ARRAY |
    FORMAT_MESSAGE_ALLOCATE_BUFFER,
    NULL,
    lastError,
    0,
    (LPWSTR)&errorString,
    0,
    NULL
  );

  OutputDebugStringW(errorString);
#endif
}

void PlatformDebugPrint(const char* message) {
#ifndef NDEBUG
  OutputDebugStringA(message);
#else
  (void)message;
#endif
}

}

#endif
