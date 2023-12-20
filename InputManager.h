#pragma once

#include "ZBaseTypes.h"

#include "Array.h"
#include "Delegate.h"
#include "FixedArray.h"

namespace ZSharp {
class InputManager final {
  public:

  BroadcastDelegate<uint8> OnKeyDownDelegate;

  BroadcastDelegate<uint8> OnKeyUpDelegate;

  BroadcastDelegate<int32, int32, int32, int32> OnMouseMoveDelegate;

  enum class KeyState {
    Clear,
    Down,
    Up
  };

  static InputManager& Get();

  InputManager(const InputManager&) = delete;

  void Update(uint8 key, KeyState state);

  void UpdateMousePosition(int32 x, int32 y);

  void UpdateMouseState(bool pressedDown);

  void ResetMouse();

  void Process();

  bool IsMousePressed() const;

  private:
  InputManager();

  FixedArray<KeyState, max_uint8> mKeyboard;

  bool mMousePressed = false;
  int32 mOldMouseX = 0;
  int32 mOldMouseY = 0;
  int32 mCurrentMouseX = 0;
  int32 mCurrentMouseY = 0;
};
}
