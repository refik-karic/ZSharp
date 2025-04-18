#pragma once

#include "ZBaseTypes.h"
#include "PlatformDefines.h"
#include "ConsoleVariable.h"

#pragma warning(disable:4201) // nameless struct/union

namespace ZSharp {

FORCE_INLINE uint32 ColorFromRGB(uint8 R, uint8 G, uint8 B) {
  uint32 color = 0xFF00;
  color |= R;
  color <<= 8;

  color |= G;
  color <<= 8;

  color |= B;

  return color;
}

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
  
  static uint32 LerpColors(const ZColor& colorA, const ZColor& colorB, float parametricAmount);

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
  YELLOW = 0xFFFFFF00,
  BLACK = 0x0,
  GRAY = 0xFF808080,
  WHITE = 0xFFFFFFFF,
};

template<>
struct ConsoleVariableConverter<ZColor> {
  void operator()(const String& str, ZColor& var) const {
    String ignoredcase(str);
    ignoredcase.ToLower();

    if (ignoredcase == "red") {
      var = ZColor(ZColors::RED);
    }
    else if (ignoredcase == "blue") {
      var = ZColor(ZColors::BLUE);
    }
    else if (ignoredcase == "green") {
      var = ZColor(ZColors::GREEN);
    }
    else if (ignoredcase == "orange") {
      var = ZColor(ZColors::ORANGE);
    }
    else if (ignoredcase == "yellow") {
      var = ZColor(ZColors::YELLOW);
    }
    else if (ignoredcase == "black") {
      var = ZColor(ZColors::BLACK);
    }
    else if (ignoredcase == "gray") {
      var = ZColor(ZColors::GRAY);
    }
    else if (ignoredcase == "white") {
      var = ZColor(ZColors::WHITE);
    }
  }
};

}
