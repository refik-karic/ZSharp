#if defined(_WIN64)

#define _CRT_SECURE_NO_WARNINGS 1

#include "PlatformFile.h"

#define WIN32_LEAN_AND_MEAN
#define _AMD64_

#include <fileapi.h>
#include <handleapi.h>
#include <ShlObj_core.h>

#include <stdlib.h>

namespace ZSharp { 

DWORD SetFlags(size_t inFlags) {
  DWORD winFlags = 0;
  
  if (inFlags & static_cast<size_t>(FileFlags::READ)) {
    winFlags |= GENERIC_READ;
  }
  else if (inFlags & static_cast<size_t>(FileFlags::WRITE)) {
    winFlags |= GENERIC_WRITE;
  }

  return winFlags;
}

void* PlatformOpenFile(FileString& filename, size_t flags) {
  DWORD winFlags = SetFlags(flags);
  DWORD shareFlags = (flags & static_cast<size_t>(FileFlags::READ)) ? FILE_SHARE_READ : 0;

  HANDLE fileHandle = CreateFileA(filename.GetAbsolutePath().Str(),
    winFlags,
    shareFlags,
    NULL,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    NULL
  );

  bool isWriting = (flags && static_cast<size_t>(FileFlags::WRITE));
  if (fileHandle == NULL && isWriting) {
    fileHandle = CreateFileA(filename.GetAbsolutePath().Str(),
      winFlags,
      shareFlags,
      NULL,
      CREATE_NEW,
      FILE_ATTRIBUTE_NORMAL,
      NULL
    );
  }

  return fileHandle;
}

void PlatformCloseFile(void* handle) {
  CloseHandle(handle);
}

size_t PlatformReadFile(void* handle, void* buffer, size_t length) {
  DWORD bytesRead = 0;
  if (ReadFile(handle, buffer, (DWORD)length, &bytesRead, NULL)) {
    return static_cast<size_t>(bytesRead);
  }
  else {
    return 0;
  }
}

size_t PlatformWriteFile(void* handle, const void* buffer, size_t length) {
  DWORD bytesWritten = 0;
  if (WriteFile(handle, buffer, (DWORD)length, &bytesWritten, NULL)) {
    return bytesWritten;
  }
  else {
    return 0;
  }
}

bool PlatformFileFlush(void* handle) {
  return FlushFileBuffers(handle);
}

FileString PlatformGetUserDesktopPath() {
  wchar_t* pathResult = nullptr;
  HRESULT result = SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &pathResult);
  if (result == S_OK) {
    char convertedPath[_MAX_PATH];
    convertedPath[0] = NULL;
    wcstombs(convertedPath, pathResult, sizeof(convertedPath));
    return FileString(convertedPath);
  }
  else {
    return FileString("");
  }
}

FileString PlatformGetExecutableDirectory() {
  char path[_MAX_PATH];
  if (GetModuleFileNameA(NULL, path, _MAX_PATH)) {
    FileString receivedPath(path);
    receivedPath.SetFilename("");
    return receivedPath;
  }
  else {
    return FileString("");
  }
}

}

#endif
