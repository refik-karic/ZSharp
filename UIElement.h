#pragma once

#include "ZBaseTypes.h"
#include "UIBase.h"

namespace ZSharp {

/*
Element is a base class for things like Labels or Buttons.
*/
class UIElement : public UIBase {
  public:

  UIElement(size_t width, size_t height, const String& name);

  private:
};

}
