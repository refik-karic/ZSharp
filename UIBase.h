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
  Top,
  Center,
  Bottom,
  Fill
};

class UIBase {
  public:

  UIBase();

  UIBase(size_t width, size_t height, const String& name);

  virtual void Layout(size_t x, size_t y);

  virtual void HitTest(int32 x, int32 y);

  virtual void Draw(uint8* screen, size_t width, size_t height);

  bool operator==(const UIBase& rhs) const;

  size_t& Width();

  size_t& Height();

  UIHorizontalAlignment& HorizontalAlignment();

  UIVerticalAlignment& VerticalAlignment();

  protected:
  size_t mWidth;
  size_t mHeight;
  size_t mX;
  size_t mY;
  String mName;
  UIHorizontalAlignment mHorizontalAlignment = UIHorizontalAlignment::Fill;
  UIVerticalAlignment mVerticalAlignment = UIVerticalAlignment::Fill;

  bool mMouseOver;
};

}
