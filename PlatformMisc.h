#pragma once

#include "ZBaseTypes.h"
#include "Array.h"
#include "ZString.h"

namespace ZSharp {

String PlatformGetBuildType();

String PlatformGetToolchain();

String PlatformGetUsername();

String PlatformGetMachineName();

Array<String> PlatformEnumDisplayInfo();

}
