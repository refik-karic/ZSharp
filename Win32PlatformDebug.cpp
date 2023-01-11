#ifdef PLATFORM_WINDOWS

#include "PlatformDebug.h"

#include "ZAssert.h"
#include "Win32PlatformHeaders.h"

#include <consoleapi.h>
#include <consoleapi3.h>
#include <debugapi.h>
#include <errhandlingapi.h>
#include <WinBase.h>

namespace ZSharp {

bool PlatformHasConsole() {
  HWND handle = GetConsoleWindow();
  return (handle != NULL) && (handle != INVALID_HANDLE_VALUE);
}

void PlatformWriteConsole(const String& msg) {
  HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
  if ((handle != NULL) && (handle != INVALID_HANDLE_VALUE)) {
    DWORD numWritten;
    if (!WriteConsoleA(handle, msg.Str(), (DWORD)msg.Length(), &numWritten, NULL)) {
      PlatformDebugPrintLastError();
      return;
    }

    ZAssert(numWritten == ((DWORD)msg.Length()));
  }
}

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
