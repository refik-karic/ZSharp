#include "DevConsole.h"

#include "DebugText.h"
#include "Delegate.h"
#include "InputManager.h"
#include "PlatformMemory.h"
#include "PlatformIntrinsics.h"
#include "ZConfig.h"
#include "ZColor.h"
#include "Win32PlatformApplication.h"
#include "ScopedTimer.h"

#include <ctype.h>
#include <cstring>

namespace ZSharp {
DevConsole::DevConsole() {
  InputManager& inputManager = InputManager::Get();
  inputManager.OnKeyDownDelegate.Add(Delegate<uint8>::FromMember<DevConsole, &DevConsole::OnKeyDown>(this));
  inputManager.OnKeyUpDelegate.Add(Delegate<uint8>::FromMember<DevConsole, &DevConsole::OnKeyUp>(this));

  ZConfig& config = ZConfig::Get();
  OnResize(config.GetViewportWidth().Value(), config.GetViewportHeight().Value());

  Win32PlatformApplication::OnWindowSizeChangedDelegate.Add(Delegate<size_t, size_t>::FromMember<DevConsole, &DevConsole::OnResize>(this));
}

DevConsole::~DevConsole() {
  InputManager& inputManager = InputManager::Get();
  inputManager.OnKeyDownDelegate.Remove(Delegate<uint8>::FromMember<DevConsole, &DevConsole::OnKeyDown>(this));
  inputManager.OnKeyUpDelegate.Remove(Delegate<uint8>::FromMember<DevConsole, &DevConsole::OnKeyUp>(this));

  Win32PlatformApplication::OnWindowSizeChangedDelegate.Remove(Delegate<size_t, size_t>::FromMember<DevConsole, &DevConsole::OnResize>(this));

  if (mScreen != nullptr) {
    PlatformAlignedFree(mScreen);
  }
}

bool DevConsole::IsOpen() const {
  return mOpen;
}

void DevConsole::Draw(uint32* buffer) {
  NamedScopedTimer(DevConsoleDraw);

  ZColor clearColor(ZColors::BLACK);
  ZColor textColor(ZColors::WHITE);
  Aligned_Memset(mScreen, clearColor.Color(), mWidth * mHeight * sizeof(uint32));

  const String message(mActiveBuffer, 0, mCaret);
  {
    ZColor historyColor(ZColors::GRAY);
    List<String>::Iterator iter = mHistory.rbegin();
    int32 currentHeight = (int32)mHeight - 40;

    for (size_t i = 0; i < mHistory.Size(); ++i) {
      if (currentHeight > 0) {
        DrawText(*iter, 10, currentHeight, (uint8*)mScreen, mWidth, historyColor);
        currentHeight -= 20;
        iter--;
      }
      else {
        break;
      }
    }
  }

  DrawText(message, 10, mHeight - 20, (uint8*)mScreen, mWidth, textColor);

  for (size_t y = 0; y < mHeight; ++y) {
    for (size_t x = 0; x < mWidth; ++x) {
      size_t index = (y * mWidth) + x;

      ZColor currentColor(mScreen[index]);
      ZColor otherColor(buffer[index]);
      uint8 lR = (uint8)((float)currentColor.R() * mOpacity);
      uint8 lG = (uint8)((float)currentColor.G() * mOpacity);
      uint8 lB = (uint8)((float)currentColor.B() * mOpacity);

      uint8 rR = (uint8)((float)otherColor.R() * (1.f - mOpacity));
      uint8 rG = (uint8)((float)otherColor.G() * (1.f - mOpacity));
      uint8 rB = (uint8)((float)otherColor.B() * (1.f - mOpacity));

      ZColor finalColor((uint8)(lR + rR), (uint8)(lG + rG), (uint8)(lB + rB));
      buffer[index] = finalColor.Color();
    }
  }
  //Unaligned_BlendDevConsole((float*)mScreen, (float*)buffer, mWidth, mHeight, mOpacity);
}

void DevConsole::OnKeyDown(uint8 key) {
  if (key == '`') {
    mOpen = !mOpen;
    return;
  }

  // Backspace
  if (key == 8) {
    if (mCaret > 0) {
      --mCaret;
    }

    return;
  }

  if (key == 10 || key == 13) {
    const String message(mActiveBuffer, 0, mCaret);
    if (mHistory.Size() > 10) {
      mHistory.RemoveFront();
    }

    mHistory.Add(message);
    mCaret = 0;
    return;
  }

  if (isalpha(key) || isdigit(key) || isspace(key) || key == ',' || key == '.') {
    mActiveBuffer[mCaret++] = key;
  }
}

void DevConsole::OnKeyUp(uint8 key) {
  (void)key;
}

void DevConsole::OnResize(size_t width, size_t height) {
  if (mScreen != nullptr) {
    PlatformAlignedFree(mScreen);
  }

  mWidth = width;
  mHeight = height / 3;
  mScreen = (uint32*)PlatformAlignedMalloc(mWidth * mHeight * sizeof(uint32), 64);
}

}
