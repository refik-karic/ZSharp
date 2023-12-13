#pragma once

#include "ZBaseTypes.h"
#include "ZString.h"

#include "ZColor.h"

namespace ZSharp {

void DrawText(const String& message, size_t x, size_t y, uint8* buffer, size_t width, const ZColor& color);

}
