#pragma once

#include "ZBaseTypes.h"
#include "ZString.h"

namespace ZSharp {

enum class UIHorizontalAlignment {
  Left,
  Center,
  Right,
  Fill
};

enum class UIVerticalAlignment {
  Left,
  Center,
  Right,
  Fill
};

class UIBase {
  public:

  UIBase();

  UIBase(size_t width, size_t height, const String& name);

  virtual void Draw(uint8* screen, size_t width, size_t height, size_t offset);

  bool operator==(const UIBase& rhs) const;

  size_t& Width();

  size_t& Height();

  UIHorizontalAlignment& HorizontalAlignment();

  UIVerticalAlignment& VerticalAlignment();

  protected:
  size_t mWidth;
  size_t mHeight;
  String mName;
  UIHorizontalAlignment mHorizontalAlignment = UIHorizontalAlignment::Fill;
  UIVerticalAlignment mVerticalAlignment = UIVerticalAlignment::Fill;
};

}
