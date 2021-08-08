#pragma once

#include <cstdint>

namespace ZSharp {

union ZColor {
  std::uint32_t Color;
  std::uint8_t A;
  std::uint8_t R;
  std::uint8_t G;
  std::uint8_t B;
};

enum ZColors : std::uint32_t {
  RED = 0xFFFF0000,
  BLUE = 0xFF0000FF,
  GREEN = 0xFF00FF00
};

}
