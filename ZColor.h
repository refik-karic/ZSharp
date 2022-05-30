#pragma once

#include "ZBaseTypes.h"

namespace ZSharp {

union ZColor {
  uint32 Color;
  uint8 A;
  uint8 R;
  uint8 G;
  uint8 B;
};

enum ZColors : uint32 {
  RED = 0xFFFF0000,
  BLUE = 0xFF0000FF,
  GREEN = 0xFF00FF00,
  WHITE = 0xFFFFFFFF
};

}
