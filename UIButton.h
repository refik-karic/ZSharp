#pragma once

#include "ZBaseTypes.h"
#include "UIElement.h"
#include "ZColor.h"
#include "ZString.h"

namespace ZSharp {

class UIButton : public UIElement {
  public:

  UIButton(size_t width, size_t height, const String& name);

  virtual void Draw(uint8* screen, size_t width, size_t height) override;

  private:
  String mText;
  ZColor mBackground;
};

}
