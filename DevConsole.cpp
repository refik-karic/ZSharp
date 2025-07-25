#include "DevConsole.h"

#include "ConsoleVariable.h"
#include "DebugText.h"
#include "Delegate.h"
#include "PlatformMemory.h"
#include "PlatformIntrinsics.h"
#include "PlatformMisc.h"
#include "ZConfig.h"
#include "ZColor.h"
#include "PlatformApplication.h"
#include "ScopedTimer.h"

#include <ctype.h>
#include <cstring>

namespace ZSharp {

DevConsole::DevConsole() {
  if (PlatformGetBuildType() == "Release") {
    return;
  }

  mActiveBuffer = (int8*)PlatformMalloc(512);

  InputManager* inputManager = GlobalInputManager;
  inputManager->OnKeyDownDelegate.Add(Delegate<uint8>::FromMember<DevConsole, &DevConsole::OnKeyDown>(this));
  inputManager->OnKeyUpDelegate.Add(Delegate<uint8>::FromMember<DevConsole, &DevConsole::OnKeyUp>(this));
  inputManager->OnMiscKeyDownDelegate.Add(Delegate<MiscKey>::FromMember<DevConsole, &DevConsole::OnMiscKeyDown>(this));
  inputManager->OnMiscKeyUpDelegate.Add(Delegate<MiscKey>::FromMember<DevConsole, &DevConsole::OnMiscKeyUp>(this));

  ZConfig* config = GlobalConfig;
  OnResize(config->GetViewportWidth().Value(), config->GetViewportHeight().Value());

  OnWindowSizeChangedDelegate().Add(Delegate<size_t, size_t>::FromMember<DevConsole, &DevConsole::OnResize>(this));
}

DevConsole::~DevConsole() {
  if (PlatformGetBuildType() == "Release") {
    return;
  }

  if (mActiveBuffer != nullptr) {
    PlatformFree(mActiveBuffer);
  }

  InputManager* inputManager = GlobalInputManager;
  inputManager->OnKeyDownDelegate.Remove(Delegate<uint8>::FromMember<DevConsole, &DevConsole::OnKeyDown>(this));
  inputManager->OnKeyUpDelegate.Remove(Delegate<uint8>::FromMember<DevConsole, &DevConsole::OnKeyUp>(this));
  inputManager->OnMiscKeyDownDelegate.Remove(Delegate<MiscKey>::FromMember<DevConsole, &DevConsole::OnMiscKeyDown>(this));
  inputManager->OnMiscKeyUpDelegate.Remove(Delegate<MiscKey>::FromMember<DevConsole, &DevConsole::OnMiscKeyUp>(this));

  OnWindowSizeChangedDelegate().Remove(Delegate<size_t, size_t>::FromMember<DevConsole, &DevConsole::OnResize>(this));

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
  ZColor historyColor(ZColors::GRAY);
  Aligned_Memset(mScreen, clearColor.Color(), mWidth * mHeight * sizeof(uint32));

  {
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
  const String formattedSuggestion(String::FromFormat("> {0}", mLastSuggestion));
  DrawText(formattedSuggestion, 10, mHeight - 15, (uint8*)mScreen, mWidth, historyColor);
  DrawText(formattedMessage, 10, mHeight - 15, (uint8*)mScreen, mWidth, textColor);

  BlendBuffersImpl(mScreen, buffer, mWidth, mHeight, mOpacity);
}

void DevConsole::OnKeyDown(uint8 key) {
  if (key == '`') {
    mOpen = !mOpen;

    if (mOpen) {
      UpdateSuggestions();
    }

    return;
  }

  if (!mOpen) {
    return;
  }

  mActiveBuffer[mCaret++] = key;

  String substring((const char*)mActiveBuffer, 0, mCaret);
  Pair<Trie::Iterator, Trie::Iterator> lastSuggestion = mSuggestions.NextWords(substring);
  mLastSuggestion = *(lastSuggestion.mKey);
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

      String substring((const char*)mActiveBuffer, 0, mCaret);
      Pair<Trie::Iterator, Trie::Iterator> lastSuggestion = mSuggestions.NextWords(substring);
      mLastSuggestion = *(lastSuggestion.mKey);
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
  else if (key == MiscKey::RIGHT_ARROW) {
    size_t caretPos = mLastSuggestion.Length();
    memcpy(mActiveBuffer, mLastSuggestion.Str(), caretPos);
    mCaret = caretPos;
    return;
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
      if (GlobalConsoleCommands().HasKey(commandName)) {
        Delegate<const String&>& func = GlobalConsoleCommands()[commandName];
        func(commandValue);
      }
    }
    else {
      if (GlobalConsoleCommandsValueless().HasKey(message)) {
        Delegate<void>& func = GlobalConsoleCommandsValueless()[message];
        func();
      }
    }

    mHistory.Add(message);
    mCaret = 0;
    mHistoryPos = 0;
    mLastSuggestion = "";
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
  mScreen = (uint32*)PlatformAlignedMalloc(mWidth * mHeight * sizeof(uint32), PlatformAlignmentGranularity());
}

void DevConsole::UpdateSuggestions() {
  for (Pair<String, Delegate<const String&>>& kvp : GlobalConsoleCommands()) {
    mSuggestions.Add(String::FromFormat("{0} _", kvp.mKey));
  }
  
  for (Pair<String, Delegate<void>>& kvp : GlobalConsoleCommandsValueless()) {
    mSuggestions.Add(kvp.mKey);
  }
}

}
