#pragma once

#include "ZBaseTypes.h"
#include "ZString.h"

namespace ZSharp {

bool PlatformHasConsole();

void PlatformWriteConsole(const String& msg);

void PlatformDebugPrintLastError();

void PlatformDebugPrint(const char* message);

}
