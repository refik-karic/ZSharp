#pragma once

#include <array>
#include <vector>

#include <cstdint>

#include "IInputListener.h"

namespace ZSharp {
class InputManager final {
  public:

  enum class KeyState {
    Clear,
    Down,
    Up
  };

  static InputManager* GetInstance();

  void Update(std::uint8_t key, KeyState state);

  void Process();

  void Register(IInputListener* inputListener);

  void Unregister(IInputListener* inputListener);

  private:
  InputManager();

  std::vector<IInputListener*> mListenerList;

  std::array<KeyState, UINT8_MAX> mKeyboard;
};
}
