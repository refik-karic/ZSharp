#include "ZColor.h"

namespace ZSharp {
void ZColor::FloatToRGB(const float R, const float G, const float B) {
  mColor = 0xFF000000;
  mColor |= (0x00FF0000 & (static_cast<uint8>((uint32)R * (uint32)0xFFU) << 16));
  mColor |= (0x0000FF00 & (static_cast<uint8>((uint32)G * (uint32)0xFFU) << 8));
  mColor |= (0x000000FF & static_cast<uint8>((uint32)B * (uint32)0xFFU));
}
}
