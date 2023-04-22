#pragma once

#include "ZBaseTypes.h"
#include "Array.h"
#include "ZString.h"

namespace ZSharp {

String PlatformGetUsername();

String PlatformGetMachineName();

Array<String> PlatformEnumDisplayInfo();

}
