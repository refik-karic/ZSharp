#pragma once

#include "ZBaseTypes.h"

namespace ZSharp {

class ZColor {
  public:

  ZColor() {
  }

  ZColor(uint32 color) : mColor(color) {
  }

  ZColor(const float R, const float G, const float B) {
    FloatToRGB(R, G, B);
  }

  ZColor(const ZColor& rhs) : mColor(rhs.mColor) {
  }

  uint32 Color() const {
    return mColor;
  }

  void FloatToRGB(const float R, const float G, const float B);

  private:
  uint32 mColor = 0x0U;
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
