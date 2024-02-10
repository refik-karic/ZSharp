#include "InputManager.h"

namespace ZSharp {
InputManager::InputManager() {
  mKeyboard.Fill(KeyState::Clear);
  mMiscKeys.Fill(KeyState::Clear);
}

InputManager& InputManager::Get() {
  static InputManager inputManager;
  return inputManager;
}

void InputManager::Update(uint8 key, InputManager::KeyState state) {
  mKeyboard[key] = state;
}

void InputManager::UpdateMiscKey(MiscKey key, InputManager::KeyState state) {
  mMiscKeys[static_cast<size_t>(key)] = state;
}

void InputManager::UpdateMousePosition(int32 x, int32 y) {
  if (mMousePressed) {
    mCurrentMouseX = x;
    mCurrentMouseY = y;
  }
  else {
    mOldMouseX = x;
    mOldMouseY = y;
    mCurrentMouseX = x;
    mCurrentMouseY = y;
  }
}

void InputManager::UpdateMouseState(bool pressedDown) {
  mMousePressed = pressedDown;
}

void InputManager::Process() {
  for (uint8 i = 0; i < mKeyboard.Size(); ++i) {
    switch (mKeyboard[i]) {
      case KeyState::Clear:
        break;
      case KeyState::Down:
        OnKeyDownDelegate.Broadcast(i);
        break;
      case KeyState::Up:
        OnKeyUpDelegate.Broadcast(i);
        break;
    }
  }

  for (uint8 i = 0; i < mMiscKeys.Size(); ++i) {
    switch (mMiscKeys[i]) {
      case KeyState::Clear:
        break;
      case KeyState::Down:
        OnMiscKeyDownDelegate.Broadcast(static_cast<MiscKey>(i));
        break;
      case KeyState::Up:
        OnMiscKeyUpDelegate.Broadcast(static_cast<MiscKey>(i));
        break;
    }
  }

  if (mMousePressed) {
    OnMouseDragDelegate.Broadcast(mOldMouseX, mOldMouseY, mCurrentMouseX, mCurrentMouseY);
    UpdateMousePosition(mCurrentMouseX, mCurrentMouseY);
  }

  OnMouseMoveDelegate.Broadcast(mCurrentMouseX, mCurrentMouseY);

  mKeyboard.Fill(KeyState::Clear);
  mMiscKeys.Fill(KeyState::Clear);
}

bool InputManager::IsMousePressed() const {
  return mMousePressed;
}
}
