#include "ZColor.h"

#include "ZAssert.h"
#include "CommonMath.h"

namespace ZSharp {
ZColor::ZColor() : mColor(0) {
}

ZColor::ZColor(uint32 color) : mColor(color) { 
}

ZColor::ZColor(uint8 R, uint8 G, uint8 B) : mA(0xFF), mR(R), mG(G), mB(B) {
}

ZColor::ZColor(const float R, const float G, const float B)
  : mA(0xFF), mR((uint8)(R * 255.f)), mG((uint8)(G * 255.f)), mB((uint8)(B * 255.f)) {
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

uint32 ColorFromRGB(uint8 R, uint8 G, uint8 B) {
  uint32 color = 0xFF00;
  color |= R;
  color <<= 8;

  color |= G;
  color <<= 8;

  color |= B;

  return color;
}

}
