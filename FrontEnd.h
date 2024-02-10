#pragma once

#include "ZBaseTypes.h"
#include "UIFrame.h"

namespace ZSharp {

class FrontEnd final {
  public:

  FrontEnd();

  ~FrontEnd();

  void Load();

  void Unload();

  void Tick();

  bool IsVisible() const;

  void Draw(uint8* screen, size_t width, size_t height);

  private:
  bool mVisible = false;
  UIFrame* mFrame = nullptr;

  void OnResize(size_t width, size_t height);

  void OnMouseMove(int32 x, int32 y);
};

}
