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

  protected:
};

}
