#pragma once

#include "ZBaseTypes.h"
#include "UIBase.h"

namespace ZSharp {

/*
Container is a base class for things like Grid or LinearPanel.
*/
class UIContainer : public UIBase {
  public:

  UIContainer(size_t width, size_t height, const String& name);

  virtual void Draw(uint8* screen, size_t width, size_t height, size_t offset) override {
    (void)screen;
    (void)width;
    (void)height;
    (void)offset;
  }

  protected:
};

}
