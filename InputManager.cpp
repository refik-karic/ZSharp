#include "InputManager.h"

namespace ZSharp {
InputManager::InputManager()
  : mListenerList() {
  mKeyboard.fill(KeyState::Clear);
}

InputManager& InputManager::GetInstance() {
  static InputManager inputManager;
  return inputManager;
}

void InputManager::Update(uint8_t key, InputManager::KeyState state) {
  mKeyboard[key] = state;
}

void InputManager::UpdateMousePosition(int32_t x, int32_t y) {
  if (mMousePressed) {
    mCurrentMouseX = x;
    mCurrentMouseY = y;
  }
  else {
    mOldMouseX = x;
    mOldMouseY = y;
  }
}

void InputManager::UpdateMouseState(bool pressedDown) {
  mMousePressed = pressedDown;
}

void InputManager::ResetMouse() {
  mMousePressed = false;
  mOldMouseX = 0;
  mOldMouseY = 0;
  mCurrentMouseX = 0;
  mCurrentMouseY = 0;
}

void InputManager::Process() {
  for (uint8_t i = 0; i < mKeyboard.size(); ++i) {
    switch (mKeyboard[i]) {
      case KeyState::Clear:
        break;
      case KeyState::Down:
        for (IInputListener* listener : mListenerList) {
          if (listener != nullptr) {
            listener->OnKeyDown(i);
          }
        }
        break;
      case KeyState::Up:
        for (IInputListener* listener : mListenerList) {
          if (listener != nullptr) {
            listener->OnKeyUp(i);
          }
        }
        break;
    }
  }

  if (mMousePressed) {
    for (IInputListener* listener : mListenerList) {
      if (listener != nullptr) {
        listener->OnMouseMove(mOldMouseX, mOldMouseY, mCurrentMouseX, mCurrentMouseY);
      }
    }
  }

  mKeyboard.fill(KeyState::Clear);
}

void InputManager::Register(IInputListener* inputListener) {
  mListenerList.PushBack(inputListener);
}

void InputManager::Unregister(IInputListener* inputListener) {
  for (size_t i = 0; i < mListenerList.Size(); ++i) {
    IInputListener* listener = mListenerList[i];
    
    if (listener == inputListener) {
      mListenerList[i] = nullptr;
      break;
    }
  }
}

bool InputManager::IsMousePressed() const {
  return mMousePressed;
}
}
