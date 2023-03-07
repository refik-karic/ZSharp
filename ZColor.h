#pragma once

#include "ZBaseTypes.h"

#pragma warning(disable:4201) // nameless struct/union

namespace ZSharp {

class ZColor {
  public:

  ZColor();

  ZColor(uint32 color);

  ZColor(uint8 R, uint8 G, uint8 B);

  ZColor(const float R, const float G, const float B);

  ZColor(const ZColor& rhs);

  ZColor(const ZColor& colorA, const ZColor& colorB, float parametricAmount);

  uint32 Color() const;

  void FloatToRGB(const float R, const float G, const float B);
  
  uint8 A() const;
  uint8 R() const;
  uint8 G() const;
  uint8 B() const;

  private:
  union {
    uint32 mColor;
    struct {
      uint8 mB;
      uint8 mG;
      uint8 mR;
      uint8 mA;
    };
  };
};

enum ZColors : uint32 {
  RED = 0xFFFF0000,
  BLUE = 0xFF0000FF,
  GREEN = 0xFF00FF00,
  ORANGE = 0xFFFF8000,
  BLACK = 0x0,
  WHITE = 0xFFFFFFFF,
};


}
