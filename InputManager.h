#pragma once

#include "ZBaseTypes.h"

#include "Array.h"
#include "Delegate.h"
#include "FixedArray.h"

namespace ZSharp {

enum class MiscKey : size_t {
  UP_ARROW,
  LEFT_ARROW,
  DOWN_ARROW,
  RIGHT_ARROW,
  ENTER,
  BACKSPACE,
  NUM_KEYS
};

class InputManager final {
  public:

  BroadcastDelegate<uint8> OnKeyDownDelegate;

  BroadcastDelegate<uint8> OnAsyncKeyDownDelegate;

  BroadcastDelegate<uint8> OnKeyUpDelegate;

  BroadcastDelegate<MiscKey> OnMiscKeyDownDelegate;

  BroadcastDelegate<MiscKey> OnAsyncMiscKeyDownDelegate;

  BroadcastDelegate<MiscKey> OnMiscKeyUpDelegate;

  BroadcastDelegate<int32, int32, int32, int32> OnMouseDragDelegate;

  BroadcastDelegate<int32, int32, bool> OnMouseMoveDelegate;

  enum class KeyState {
    Clear,
    Down,
    Up,
    Stale // Indicates that a key is still down after an OnDown callback.
  };

  InputManager();

  InputManager(const InputManager&) = delete;

  void Update(uint8 key, KeyState state);

  void UpdateMiscKey(MiscKey key, KeyState state);

  void UpdateMousePosition(int32 x, int32 y);

  void UpdateMouseState(bool pressedDown);

  void Process();

  bool IsMousePressed() const;

  KeyState GetKeyState(uint8 key);

  KeyState GetMiscKeyState(MiscKey key);

  private:

  FixedArray<KeyState, max_uint8> mKeyboard;
  FixedArray<KeyState, static_cast<size_t>(MiscKey::NUM_KEYS)> mMiscKeys;

  bool mMousePressed = false;
  int32 mOldMouseX = 0;
  int32 mOldMouseY = 0;
  int32 mCurrentMouseX = 0;
  int32 mCurrentMouseY = 0;
};

extern InputManager* GlobalInputManager;

}
