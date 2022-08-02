#pragma once

#include "FileString.h"

namespace ZSharp {

enum class FileFlags : size_t {
  READ = 1 << 0,
  WRITE = 1 << 1
};

void* PlatformOpenFile(FileString& filename, size_t flags);

void PlatformCloseFile(void* handle);

size_t PlatformReadFile(void* handle, void* buffer, size_t length);

size_t PlatformWriteFile(void* handle, const void* buffer, size_t length);

bool PlatformFileFlush(void* handle);

FileString PlatformGetUserDesktopPath();

}
