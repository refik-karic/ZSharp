#if defined(_WIN64)

#define _CRT_SECURE_NO_WARNINGS 1

#include "PlatformFile.h"
#include "PlatformLogging.h"

#include <ShlObj_core.h>

#include "Win32PlatformHeaders.h"

#include <fileapi.h>
#include <handleapi.h>

#include <stdlib.h>

namespace ZSharp { 

struct PlatformFileHandle {
  HANDLE fileHandle = nullptr;
};

struct PlatformMemoryMappedFileHandle {
  PlatformFileHandle* genericFileHandle = nullptr;
  HANDLE memoryMappedHandle = nullptr;
  void* mappedData = nullptr;
};

DWORD SetFlags(size_t inFlags) {
  DWORD winFlags = 0;
  
  if (inFlags & static_cast<size_t>(FileFlags::READ)) {
    winFlags |= GENERIC_READ;
  }
  
  if (inFlags & static_cast<size_t>(FileFlags::WRITE)) {
    winFlags |= GENERIC_WRITE;
  }

  return winFlags;
}

DWORD SetMemoryMappedPageFlags(size_t inFlags) {
  DWORD winFlags = 0;

  if ((inFlags & static_cast<size_t>(FileFlags::READ)) 
    && (inFlags & static_cast<size_t>(FileFlags::WRITE))) {
    winFlags |= PAGE_READWRITE;
  }
  else if (inFlags & static_cast<size_t>(FileFlags::READ)) {
    winFlags |= PAGE_READONLY;
  }

  return winFlags;
}

DWORD SetMemoryMappedFlags(size_t inFlags) {
  DWORD winFlags = 0;

  if (inFlags & static_cast<size_t>(FileFlags::WRITE)) {
    winFlags |= FILE_MAP_WRITE;
  }
  else if (inFlags & static_cast<size_t>(FileFlags::READ)) {
    winFlags |= FILE_MAP_READ;
  }

  return winFlags;
}

PlatformFileHandle* PlatformOpenFile(const FileString& filename, size_t flags) {
  DWORD winFlags = SetFlags(flags);
  DWORD shareFlags = (flags & static_cast<size_t>(FileFlags::READ)) ? FILE_SHARE_READ : 0;
  if (flags & static_cast<size_t>(FileFlags::WRITE)) {
    shareFlags = 0;
  }

  HANDLE fileHandle = CreateFileA(filename.GetAbsolutePath().Str(),
    winFlags,
    shareFlags,
    NULL,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    NULL
  );

  bool isWriting = (flags & static_cast<size_t>(FileFlags::WRITE));
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

  if (fileHandle != nullptr) {
    PlatformFileHandle* outHandle = new PlatformFileHandle;
    outHandle->fileHandle = fileHandle;
    return outHandle;
  }
  else {
    return nullptr;
  }
}

void PlatformCloseFile(PlatformFileHandle* handle) {
  CloseHandle(handle->fileHandle);
  delete handle;
}

size_t PlatformReadFile(PlatformFileHandle* handle, void* buffer, size_t length) {
  DWORD bytesRead = 0;
  if (ReadFile(handle->fileHandle, buffer, (DWORD)length, &bytesRead, NULL)) {
    return static_cast<size_t>(bytesRead);
  }
  else {
    return 0;
  }
}

size_t PlatformWriteFile(PlatformFileHandle* handle, const void* buffer, size_t length) {
  DWORD bytesWritten = 0;
  if (WriteFile(handle->fileHandle, buffer, (DWORD)length, &bytesWritten, NULL)) {
    return bytesWritten;
  }
  else {
    return 0;
  }
}

bool PlatformFileFlush(PlatformFileHandle* handle) {
  return FlushFileBuffers(handle->fileHandle);
}

void* PlatformOpenMemoryMapFile(PlatformFileHandle* handle, size_t flags, PlatformMemoryMappedFileHandle*& outMemoryMappedFileHandle, size_t size) {
  if (handle == nullptr) {
    return nullptr;
  }

  DWORD pageFlags = SetMemoryMappedPageFlags(flags);
  DWORD fileSize = static_cast<DWORD>(size);
  HANDLE memoryMappedHandle = CreateFileMappingA(handle->fileHandle, NULL, pageFlags, HIWORD(fileSize), LOWORD(fileSize), NULL);
  if (memoryMappedHandle == nullptr) {
    PlatformDebugPrintLastError();
    PlatformCloseFile(handle);
    return nullptr;
  }

  DWORD mappedFlags = SetMemoryMappedFlags(flags);
  void* fileBuffer = MapViewOfFile(memoryMappedHandle, mappedFlags, 0, 0, size);
  if (fileBuffer == nullptr) {
    PlatformDebugPrintLastError();
    CloseHandle(memoryMappedHandle);
    PlatformCloseFile(handle);
    return nullptr;
  }

  outMemoryMappedFileHandle = new PlatformMemoryMappedFileHandle;
  outMemoryMappedFileHandle->genericFileHandle = handle;
  outMemoryMappedFileHandle->memoryMappedHandle = memoryMappedHandle;
  outMemoryMappedFileHandle->mappedData = fileBuffer;

  return fileBuffer;
}

void PlatformCloseMemoryMapFile(PlatformMemoryMappedFileHandle* memoryMappedFileHandle) {
  UnmapViewOfFile(memoryMappedFileHandle->mappedData);
  CloseHandle(memoryMappedFileHandle->memoryMappedHandle);
  delete memoryMappedFileHandle;
}

bool PlatformFlushMemoryMapFile(PlatformMemoryMappedFileHandle* memoryMappedFileHandle) {
  return FlushViewOfFile(memoryMappedFileHandle->mappedData, 0);
}

size_t PlatformGetFileSize(PlatformFileHandle* handle) {
  LARGE_INTEGER largeInt;
  largeInt.QuadPart = 0;
  if (GetFileSizeEx(handle->fileHandle, &largeInt)) {
    return static_cast<size_t>(largeInt.QuadPart);
  }
  else {
    return 0;
  }
}

bool PlatformUpdateFileAccessTime(PlatformFileHandle* handle) {
  FILETIME systemTime;
  GetSystemTimeAsFileTime(&systemTime);
  FILETIME fileTime;
  if (!FileTimeToLocalFileTime(&systemTime, &fileTime)) {
    return false;
  }

  return SetFileTime(handle->fileHandle, NULL, &fileTime, NULL);
}

bool PlatformUpdateFileModificationTime(PlatformFileHandle* handle) {
  FILETIME systemTime;
  GetSystemTimeAsFileTime(&systemTime);
  FILETIME fileTime;
  if (!FileTimeToLocalFileTime(&systemTime, &fileTime)) {
    return false;
  }

  return SetFileTime(handle->fileHandle, NULL, NULL, &fileTime);
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
