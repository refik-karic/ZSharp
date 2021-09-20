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

  void UpdateMousePosition(std::int32_t x, std::int32_t y);

  void UpdateMouseState(bool pressedDown);

  void ResetMouse();

  void Process();

  void Register(IInputListener* inputListener);

  void Unregister(IInputListener* inputListener);

  bool IsMousePressed() const;

  private:
  InputManager();

  std::vector<IInputListener*> mListenerList;

  std::array<KeyState, UINT8_MAX> mKeyboard;

  bool mMousePressed = false;
  std::int32_t mOldMouseX = 0;
  std::int32_t mOldMouseY = 0;
  std::int32_t mCurrentMouseX = 0;
  std::int32_t mCurrentMouseY = 0;
};
}
