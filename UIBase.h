#pragma once

#include "ZBaseTypes.h"
#include "ZString.h"

namespace ZSharp {

class UIBase {
  public:

  UIBase();

  UIBase(size_t width, size_t height, const String& name);

  virtual void Draw(uint8* screen, size_t width, size_t height);

  protected:
  size_t mWidth;
  size_t mHeight;
  String mName;
};

}
