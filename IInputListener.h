#pragma once

#include <cstdlib>

namespace ZSharp {

class IInputListener {
  public:

  virtual void OnKeyDown(uint8 key) = 0;

  virtual void OnKeyUp(uint8 key) = 0;

  virtual void OnMouseMove(int32 oldX, int32 oldY, int32 x, int32 y) = 0;

  private:
};

}
