#include "ZColor.h"

#include "ZAssert.h"

namespace ZSharp {
ZColor::ZColor(uint32 color) : mColor(color) { }

ZColor::ZColor(uint8 R, uint8 G, uint8 B) {
  mColor = 0xFF000000;
  mColor |= (0x00FF0000 & (static_cast<uint32>(R) << 16));
  mColor |= (0x0000FF00 & (static_cast<uint32>(G) << 8));
  mColor |= (0x000000FF & (static_cast<uint32>(B)));
}

ZColor::ZColor(const float R, const float G, const float B) {
  FloatToRGB(R, G, B);
}

void ZColor::FloatToRGB(const float R, const float G, const float B) {
  mColor = 0xFF000000;
  mColor |= (0x00FF0000 & (static_cast<uint8>(R * (float)0xFFU) << 16));
  mColor |= (0x0000FF00 & (static_cast<uint8>(G * (float)0xFFU) << 8));
  mColor |= (0x000000FF & static_cast<uint8>(B * (float)0xFFU));
}

uint8 ZColor::A() const {
  return ((mColor >> 24) & 0xFF);
}

uint8 ZColor::R() const {
  return ((mColor >> 16) & 0xFF);
}

uint8 ZColor::G() const {
  return ((mColor >> 8) & 0xFF);
}

uint8 ZColor::B() const {
  return (mColor & 0xFF);
}

}
