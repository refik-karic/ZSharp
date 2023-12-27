#include "DevConsole.h"

#include "ConsoleVariable.h"
#include "DebugText.h"
#include "Delegate.h"
#include "PlatformMemory.h"
#include "PlatformIntrinsics.h"
#include "ZConfig.h"
#include "ZColor.h"
#include "Win32PlatformApplication.h"
#include "ScopedTimer.h"

#include <ctype.h>
#include <cstring>

namespace ZSharp {

HashTable<String, Delegate<const String&>> GlobalConsoleCommands;

DevConsole::DevConsole() {
  InputManager& inputManager = InputManager::Get();
  inputManager.OnKeyDownDelegate.Add(Delegate<uint8>::FromMember<DevConsole, &DevConsole::OnKeyDown>(this));
  inputManager.OnKeyUpDelegate.Add(Delegate<uint8>::FromMember<DevConsole, &DevConsole::OnKeyUp>(this));
  inputManager.OnMiscKeyDownDelegate.Add(Delegate<MiscKey>::FromMember<DevConsole, &DevConsole::OnMiscKeyDown>(this));
  inputManager.OnMiscKeyUpDelegate.Add(Delegate<MiscKey>::FromMember<DevConsole, &DevConsole::OnMiscKeyUp>(this));

  ZConfig& config = ZConfig::Get();
  OnResize(config.GetViewportWidth().Value(), config.GetViewportHeight().Value());

  Win32PlatformApplication::OnWindowSizeChangedDelegate.Add(Delegate<size_t, size_t>::FromMember<DevConsole, &DevConsole::OnResize>(this));
}

DevConsole::~DevConsole() {
  InputManager& inputManager = InputManager::Get();
  inputManager.OnKeyDownDelegate.Remove(Delegate<uint8>::FromMember<DevConsole, &DevConsole::OnKeyDown>(this));
  inputManager.OnKeyUpDelegate.Remove(Delegate<uint8>::FromMember<DevConsole, &DevConsole::OnKeyUp>(this));
  inputManager.OnMiscKeyDownDelegate.Remove(Delegate<MiscKey>::FromMember<DevConsole, &DevConsole::OnMiscKeyDown>(this));
  inputManager.OnMiscKeyUpDelegate.Remove(Delegate<MiscKey>::FromMember<DevConsole, &DevConsole::OnMiscKeyUp>(this));

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

  {
    ZColor historyColor(ZColors::GRAY);
    List<String>::Iterator iter = mHistory.rbegin();
    int32 currentHeight = (int32)mHeight - 30;

    for (size_t i = 0; i < mHistory.Size(); ++i) {
      if (currentHeight > 0) {
        DrawText(*iter, 10, currentHeight, (uint8*)mScreen, mWidth, historyColor);
        currentHeight -= 15;
        iter--;
      }
      else {
        break;
      }
    }
  }

  const String message(mActiveBuffer, 0, mCaret);

  const String formattedMessage(String::FromFormat("> {0}_", message));
  DrawText(formattedMessage, 10, mHeight - 15, (uint8*)mScreen, mWidth, textColor);

  Unaligned_BlendBuffers(mScreen, buffer, mWidth, mHeight, mOpacity);
}

void DevConsole::OnKeyDown(uint8 key) {
  if (key == '`') {
    mOpen = !mOpen;
    return;
  }

  if (!mOpen) {
    return;
  }

  if (isalpha(key) || isdigit(key) || isspace(key) || key == ',' || key == '.') {
    mActiveBuffer[mCaret++] = key;
  }
}

void DevConsole::OnKeyUp(uint8 key) {
  (void)key;
}

void DevConsole::OnMiscKeyDown(MiscKey key) {
  if (!mOpen) {
    return;
  }

  if (key == MiscKey::BACKSPACE) {
    if (mCaret > 0) {
      --mCaret;
    }

    return;
  }

  if (key == MiscKey::UP_ARROW) {
    if (mHistory.Size() > 0) {
      List<String>::Iterator iter = mHistory.rbegin();

      for (size_t i = 0; i < mHistoryPos; ++i) {
        --iter;
      }

      const String& lastCommand = *iter;
      size_t caretPos = lastCommand.Length();
      memcpy(mActiveBuffer, lastCommand.Str(), caretPos);
      mCaret = caretPos;
      mHistoryPos = (mHistoryPos + 1) % mHistory.Size();
      return;
    }
  }

  if (key == MiscKey::ENTER) {
    const String message(mActiveBuffer, 0, mCaret);
    if (mHistory.Size() > 10) {
      mHistory.RemoveFront();
    }

    const char* commandSplit = message.FindFirst(' ');
    if (commandSplit != nullptr) {
      size_t offset = commandSplit - message.Str();
      const String commandName(message.SubStr(0, offset));
      const String commandValue(message.Str(), offset + 1, message.Length() - offset);
      if (GlobalConsoleCommands.HasKey(commandName)) {
        Delegate<const String&>& func = GlobalConsoleCommands[commandName];
        func(commandValue);
      }
    }

    mHistory.Add(message);
    mCaret = 0;
    mHistoryPos = 0;
    return;
  }
}

void DevConsole::OnMiscKeyUp(MiscKey key) {
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
