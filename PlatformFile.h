#pragma once

#include "FileString.h"

namespace ZSharp {

enum class FileFlags : size_t {
  READ = 1 << 0,
  WRITE = 1 << 1
};

struct PlatformFileHandle;

struct PlatformMemoryMappedFileHandle;

// General File I/O.
PlatformFileHandle* PlatformOpenFile(const FileString& filename, size_t flags);

void PlatformCloseFile(PlatformFileHandle* handle);

size_t PlatformReadFile(PlatformFileHandle* handle, void* buffer, size_t length);

size_t PlatformWriteFile(PlatformFileHandle* handle, const void* buffer, size_t length);

bool PlatformFileFlush(PlatformFileHandle* handle);

// Memory mapped I/O.
void* PlatformOpenMemoryMapFile(PlatformFileHandle* handle, size_t flags, PlatformMemoryMappedFileHandle*& outMemoryMappedFileHandle, size_t size);

void PlatformCloseMemoryMapFile(PlatformMemoryMappedFileHandle* memoryMappedFileHandle);

bool PlatformFlushMemoryMapFile(PlatformMemoryMappedFileHandle* memoryMappedFileHandle);

// OS file info.
size_t PlatformGetFileSize(PlatformFileHandle* handle);

bool PlatformUpdateFileAccessTime(PlatformFileHandle* handle);

bool PlatformUpdateFileModificationTime(PlatformFileHandle* handle);

FileString PlatformGetUserDesktopPath();

FileString PlatformGetExecutableDirectory();

}
