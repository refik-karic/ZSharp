#pragma once

#include <cstdlib>

namespace ZSharp {

class IInputListener {
  public:

  virtual void OnKeyDown(std::uint8_t key) = 0;

  virtual void OnKeyUp(std::uint8_t key) = 0;

  virtual void OnMouseMove(std::int32_t oldX, std::int32_t oldY, std::int32_t x, std::int32_t y) = 0;

  private:
};

}
