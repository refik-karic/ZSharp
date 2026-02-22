#pragma once

#include "ZBaseTypes.h"
#include "ZString.h"

namespace ZSharp {

void* PlatformLoadLibrary(const String& library);

void PlatformUnloadLibrary(void* handle);

void* PlatformGetLibraryFunc(void* handle, const String& funcName);

}
