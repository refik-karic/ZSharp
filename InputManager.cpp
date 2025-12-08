#include "InputManager.h"

#include "DevConsole.h"
#include "PlatformApplication.h"

namespace ZSharp {

InputManager* GlobalInputManager = nullptr;

InputManager::InputManager()
  : mKeyboard(KeyState::Clear), mMiscKeys(KeyState::Clear) {
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
  bool devConsolePriority = GlobalConsole && GlobalConsole->IsOpen();

  if (devConsolePriority) {
    for (uint8 i = 0; i < mKeyboard.Size(); ++i) {
      switch (mKeyboard[i]) {
      case KeyState::Clear:
        break;
      case KeyState::Down:
      {
        OnKeyDownDelegate.BroadcastToObject(i, GlobalConsole);
        OnAsyncKeyDownDelegate.BroadcastToObject(i, GlobalConsole);
        mKeyboard[i] = KeyState::Stale;
        break;
      }
      case KeyState::Up:
      {
        OnKeyUpDelegate.BroadcastToObject(i, GlobalConsole);
        mKeyboard[i] = KeyState::Clear;
        break;
      }
      case KeyState::Stale:
      {
        OnAsyncKeyDownDelegate.BroadcastToObject(i, GlobalConsole);
        break;
      }
      }
    }

    for (uint8 i = 0; i < mMiscKeys.Size(); ++i) {
      switch (mMiscKeys[i]) {
      case KeyState::Clear:
        break;
      case KeyState::Down:
      {
        OnMiscKeyDownDelegate.BroadcastToObject(static_cast<MiscKey>(i), GlobalConsole);
        OnAsyncMiscKeyDownDelegate.BroadcastToObject(static_cast<MiscKey>(i), GlobalConsole);
        mMiscKeys[i] = KeyState::Stale;
        break;
      }
      case KeyState::Up:
      {
        OnMiscKeyUpDelegate.BroadcastToObject(static_cast<MiscKey>(i), GlobalConsole);
        mMiscKeys[i] = KeyState::Clear;
        break;
      }
      case KeyState::Stale:
      {
        OnAsyncMiscKeyDownDelegate.BroadcastToObject(static_cast<MiscKey>(i), GlobalConsole);
        break;
      }
      }
    }
  }
  else {
    for (uint8 i = 0; i < mKeyboard.Size(); ++i) {
      switch (mKeyboard[i]) {
      case KeyState::Clear:
        break;
      case KeyState::Down:
      {
        OnKeyDownDelegate.Broadcast(i);
        OnAsyncKeyDownDelegate.Broadcast(i);
        mKeyboard[i] = KeyState::Stale;
        break;
      }
      case KeyState::Up:
      {
        OnKeyUpDelegate.Broadcast(i);
        mKeyboard[i] = KeyState::Clear;
        break;
      }
      case KeyState::Stale:
      {
        OnAsyncKeyDownDelegate.Broadcast(i);
        break;
      }
      }
    }

    for (uint8 i = 0; i < mMiscKeys.Size(); ++i) {
      switch (mMiscKeys[i]) {
      case KeyState::Clear:
        break;
      case KeyState::Down:
      {
        OnMiscKeyDownDelegate.Broadcast(static_cast<MiscKey>(i));
        OnAsyncMiscKeyDownDelegate.Broadcast(static_cast<MiscKey>(i));
        mMiscKeys[i] = KeyState::Stale;
        break;
      }
      case KeyState::Up:
      {
        OnMiscKeyUpDelegate.Broadcast(static_cast<MiscKey>(i));
        mMiscKeys[i] = KeyState::Clear;
        break;
      }
      case KeyState::Stale:
      {
        OnAsyncMiscKeyDownDelegate.Broadcast(static_cast<MiscKey>(i));
        break;
      }
      }
    }
  }

  if (mMousePressed) {
    OnMouseDragDelegate.Broadcast(mOldMouseX, mOldMouseY, mCurrentMouseX, mCurrentMouseY);
    UpdateMousePosition(mCurrentMouseX, mCurrentMouseY);
  }

  OnMouseMoveDelegate.Broadcast(mCurrentMouseX, mCurrentMouseY, mMousePressed);
}

bool InputManager::IsMousePressed() const {
  return mMousePressed;
}

InputManager::KeyState InputManager::GetKeyState(uint8 key) {
  return mKeyboard[key] == InputManager::KeyState::Stale ? InputManager::KeyState::Down : mKeyboard[key];
}

InputManager::KeyState InputManager::GetMiscKeyState(MiscKey key) {
  return mMiscKeys[static_cast<uint8>(key)] == InputManager::KeyState::Stale ? InputManager::KeyState::Down : mMiscKeys[static_cast<uint8>(key)];
}

}
