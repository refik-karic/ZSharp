#ifdef PLATFORM_WINDOWS

#include "PlatformProcess.h"
#include "Win32PlatformHeaders.h"
#include <libloaderapi.h>

namespace ZSharp {

void* PlatformLoadLibrary(const String& library) {
	return (void*)LoadLibraryA(library.Str());
}

void PlatformUnloadLibrary(void* handle) {
	if (handle) {
		FreeLibrary((HMODULE)handle);
	}
}

void* PlatformGetLibraryFunc(void* handle, const String& funcName) {
	if (handle) {
		return (void*)GetProcAddress((HMODULE)handle, funcName.Str());
	}
	else {
		return nullptr;
	}
}

}

#endif
