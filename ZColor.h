#pragma once

#include <cstdint>

namespace ZSharp {

union ZColor {
  uint32_t Color;
  uint8_t A;
  uint8_t R;
  uint8_t G;
  uint8_t B;
};

enum ZColors : uint32_t {
  RED = 0xFFFF0000,
  BLUE = 0xFF0000FF,
  GREEN = 0xFF00FF00,
  WHITE = 0xFFFFFFFF
};

}
