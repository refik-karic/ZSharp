#pragma once

#include "ZBaseTypes.h"
#include "UIElement.h"
#include "ZString.h"

namespace ZSharp {

class UILabel : public UIElement {
  public:

  UILabel(size_t width, size_t height, const String& name);

  virtual void Draw(uint8* screen, size_t width, size_t height, size_t offset) override;

  String& Text();

  private:
  String mText;
};

}
