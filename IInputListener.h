#pragma once

#include <cstdlib>

namespace ZSharp {

class IInputListener {
  public:

  virtual void OnKeyDown(uint8_t key) = 0;

  virtual void OnKeyUp(uint8_t key) = 0;

  virtual void OnMouseMove(int32_t oldX, int32_t oldY, int32_t x, int32_t y) = 0;

  private:
};

}
