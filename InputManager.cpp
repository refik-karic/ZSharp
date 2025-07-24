#include "InputManager.h"
#include "PlatformApplication.h"

namespace ZSharp {

InputManager* GlobalInputManager = nullptr;

InputManager::InputManager()
  : mKeyboard(KeyState::Clear), mMiscKeys(KeyState::Clear), mAsyncKeyboard(KeyState::Clear), mAsyncMiscKeys(KeyState::Clear) {
}

void InputManager::Update(uint8 key, InputManager::KeyState state) {
  mKeyboard[key] = state;
  mAsyncKeyboard[key] = state;
}

void InputManager::UpdateMiscKey(MiscKey key, InputManager::KeyState state) {
  mMiscKeys[static_cast<size_t>(key)] = state;
  mAsyncMiscKeys[static_cast<size_t>(key)] = state;
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

    switch (mAsyncKeyboard[i]) {
      case KeyState::Clear:
        break;
      case KeyState::Down:
        OnAsyncKeyDownDelegate.Broadcast(i);
        break;
      case KeyState::Up:
        OnAsyncKeyUpDelegate.Broadcast(i);
        mAsyncKeyboard[i] = KeyState::Clear;
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

    switch (mAsyncMiscKeys[i]) {
      case KeyState::Clear:
        break;
      case KeyState::Down:
        OnAsyncMiscKeyDownDelegate.Broadcast(static_cast<MiscKey>(i));
        break;
      case KeyState::Up:
        OnAsyncMiscKeyUpDelegate.Broadcast(static_cast<MiscKey>(i));
        mAsyncMiscKeys[i] = KeyState::Clear;
        break;
    }
  }

  if (mMousePressed) {
    OnMouseDragDelegate.Broadcast(mOldMouseX, mOldMouseY, mCurrentMouseX, mCurrentMouseY);
    UpdateMousePosition(mCurrentMouseX, mCurrentMouseY);
  }

  OnMouseMoveDelegate.Broadcast(mCurrentMouseX, mCurrentMouseY, mMousePressed);

  // NOTE: We do not clear async state on each call.
  mKeyboard.Fill(KeyState::Clear);
  mMiscKeys.Fill(KeyState::Clear);
}

bool InputManager::IsMousePressed() const {
  return mMousePressed;
}

InputManager::KeyState InputManager::GetKeyState(uint8 key) {
  return mAsyncKeyboard[key];
}

InputManager::KeyState InputManager::GetMiscKeyState(MiscKey key) {
  return mAsyncMiscKeys[static_cast<uint8>(key)];
}

}
