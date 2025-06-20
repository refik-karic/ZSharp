#include "ZColor.h"

#include "ZAssert.h"
#include "CommonMath.h"

namespace ZSharp {
ZColor::ZColor() : mColor(0) {
}

ZColor::ZColor(uint32 color) : mColor(color) { 
}

ZColor::ZColor(uint8 R, uint8 G, uint8 B) : mB(B), mG(G), mR(R), mA(0xFF) {
}

ZColor::ZColor(const float R, const float G, const float B)
  : mB((uint8)(B * 255.f)), mG((uint8)(G * 255.f)), mR((uint8)(R * 255.f)), mA(0xFF) {
}

ZColor::ZColor(const ZColor& rhs) : mColor(rhs.mColor) {
}

ZColor::ZColor(const ZColor& colorA, const ZColor& colorB, float parametricAmount) {
  mA = 0xFF;
  mR = static_cast<uint8>(Lerp(colorA.mR, colorB.mR, parametricAmount));
  mG = static_cast<uint8>(Lerp(colorA.mG, colorB.mG, parametricAmount));
  mB = static_cast<uint8>(Lerp(colorA.mB, colorB.mB, parametricAmount));
}

uint32 ZColor::Color() const {
  return mColor;
}

void ZColor::FloatToRGB(const float R, const float G, const float B) {
  mA = 0xFF;
  mR = static_cast<uint8>(R * 255.f);
  mG = static_cast<uint8>(G * 255.f);
  mB = static_cast<uint8>(B * 255.f);
}

uint32 ZColor::LerpColors(const ZColor& colorA, const ZColor& colorB, float parametricAmount) {
  uint32 color = 
  0xFF000000 |
    (
      (static_cast<uint32>(Lerp(colorA.mR, colorB.mR, parametricAmount)) << 16) |
      (static_cast<uint32>(Lerp(colorA.mG, colorB.mG, parametricAmount)) << 8) | 
      static_cast<uint32>(Lerp(colorA.mB, colorB.mB, parametricAmount))
    );
  return color;
}

uint8 ZColor::A() const {
  return mA;
}

uint8 ZColor::R() const {
  return mR;
}

uint8 ZColor::G() const {
  return mG;
}

uint8 ZColor::B() const {
  return mB;
}

}
