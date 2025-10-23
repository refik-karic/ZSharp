#pragma once

#include "ZBaseTypes.h"
#include "InputManager.h"
#include "List.h"
#include "ZString.h"
#include "Trie.h"

namespace ZSharp {

class DevConsole final {
  public:

  DevConsole();

  ~DevConsole();

  bool IsOpen() const;

  void Draw(uint32* buffer);

  private:
  uint32* mScreen = nullptr;
  size_t mWidth = 0;
  size_t mHeight = 0;
  float mOpacity = 0.6f;
  bool mOpen = false;

  size_t mCaret = 0;
  int8* mActiveBuffer;
  List<String> mHistory;
  List<String>::Iterator mHistoryPos;

  Trie mSuggestions;
  String mLastSuggestion;

  void OnKeyDown(uint8 key);

  void OnKeyUp(uint8 key);

  void OnMiscKeyDown(MiscKey key);

  void OnMiscKeyUp(MiscKey key);

  void OnResize(size_t width, size_t height);

  void UpdateSuggestions();
};

}
