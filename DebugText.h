#pragma once

#include "ZBaseTypes.h"
#include "ZString.h"

#include "Framebuffer.h"
#include "ZColor.h"

namespace ZSharp {

void DrawText(const String& message, size_t x, size_t y, Framebuffer& framebuffer, const ZColor& color);

}
