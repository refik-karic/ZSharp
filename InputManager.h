#pragma once

#include <array>

#include <cstdint>

#include "Array.h"
#include "IInputListener.h"

namespace ZSharp {
class InputManager final {
  public:

  enum class KeyState {
    Clear,
    Down,
    Up
  };

  static InputManager& GetInstance();

  InputManager(const InputManager&) = delete;

  void Update(uint8_t key, KeyState state);

  void UpdateMousePosition(int32_t x, int32_t y);

  void UpdateMouseState(bool pressedDown);

  void ResetMouse();

  void Process();

  void Register(IInputListener* inputListener);

  void Unregister(IInputListener* inputListener);

  bool IsMousePressed() const;

  private:
  InputManager();

  Array<IInputListener*> mListenerList;

  std::array<KeyState, UINT8_MAX> mKeyboard;

  bool mMousePressed = false;
  int32_t mOldMouseX = 0;
  int32_t mOldMouseY = 0;
  int32_t mCurrentMouseX = 0;
  int32_t mCurrentMouseY = 0;
};
}
