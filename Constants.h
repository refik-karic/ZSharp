#pragma once

#include <cstddef>

namespace ZSharp {

class Constants final {
  public:
  Constants() = delete;

  static constexpr float FRAMERATE_60HZ_MS = 1000.0F / 60.0F;

  static constexpr double PI = 3.14159265358979;

  static constexpr double PI_OVER_180 = PI / 180.0;

  static constexpr std::size_t MAX_VERTS_AFTER_CLIP = 2;
  static constexpr std::size_t MAX_INDICIES_AFTER_CLIP = 4;

  static constexpr std::size_t TRI_VERTS = 3;

  static constexpr std::size_t R3 = 3;
};

}
