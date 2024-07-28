#pragma once

#include "ZBaseTypes.h"
#include "UIFrame.h"

namespace ZSharp {

class FrontEnd final {
  public:

  FrontEnd();

  ~FrontEnd();

  bool IsLoaded();

  void Load();

  void Unload();

  void Tick();

  bool IsVisible() const;

  void Draw(uint8* screen, size_t width, size_t height);

  private:
  bool mLoaded = false;
  bool mVisible = false;
  UIFrame* mFrame = nullptr;

  void OnResize(size_t width, size_t height);

  void OnMouseMove(int32 x, int32 y, bool mouseDown);

  void OnStartButtonClicked();

  void OnQuitButtonClicked();

  int32 LoadBackgroundImage(const String& imageName);
};

}
