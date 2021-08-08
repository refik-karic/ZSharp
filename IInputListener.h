#pragma once

#include <cstdlib>

namespace ZSharp {

class IInputListener {
  public:

  virtual void OnKeyDown(std::uint8_t key) = 0;

  virtual void OnKeyUp(std::uint8_t key) = 0;

  private:
};

}
