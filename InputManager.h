#pragma once

#include "ZBaseTypes.h"

#include "Array.h"
#include "IInputListener.h"
#include "FixedArray.h"

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

  void Update(uint8 key, KeyState state);

  void UpdateMousePosition(int32 x, int32 y);

  void UpdateMouseState(bool pressedDown);

  void ResetMouse();

  void Process();

  void Register(IInputListener* inputListener);

  void Unregister(IInputListener* inputListener);

  bool IsMousePressed() const;

  private:
  InputManager();

  Array<IInputListener*> mListenerList;

  FixedArray<KeyState, Z_UINT8_MAX> mKeyboard;

  bool mMousePressed = false;
  int32 mOldMouseX = 0;
  int32 mOldMouseY = 0;
  int32 mCurrentMouseX = 0;
  int32 mCurrentMouseY = 0;
};
}
