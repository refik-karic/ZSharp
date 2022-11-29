#include "ZColor.h"

namespace ZSharp {
void ZColor::FloatToRGB(const float R, const float G, const float B) {
  mColor = 0xFF000000;
  mColor |= (0x00FF0000 & (static_cast<uint8>(R * (float)0xFFU) << 16));
  mColor |= (0x0000FF00 & (static_cast<uint8>(G * (float)0xFFU) << 8));
  mColor |= (0x000000FF & static_cast<uint8>(B * (float)0xFFU));
}
}
