#pragma once

#include "ZBaseTypes.h"
#include "ZString.h"
#include "Delegate.h"
#include "ZColor.h"

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

  Delegate<void> OnClickDelegate;

  UIBase();

  UIBase(size_t width, size_t height, const String& name);

  virtual void Layout(size_t x, size_t y);

  virtual void HitTest(int32 x, int32 y, bool mouseDown);

  virtual void Draw(uint8* screen, size_t width, size_t height);

  virtual void Resize(size_t width, size_t height);

  bool operator==(const UIBase& rhs) const;

  size_t& Width();

  const size_t& Width() const;

  size_t& Height();

  const size_t& Height() const;

  size_t& GridRow();

  const size_t& GridRow() const;

  size_t& GridColumn();

  const size_t& GridColumn() const;

  UIHorizontalAlignment& HorizontalAlignment();

  UIVerticalAlignment& VerticalAlignment();

  void SetMouseOver(bool state);

  protected:
  size_t mWidth;
  size_t mHeight;
  size_t mX;
  size_t mY;
  size_t mGridRow;
  size_t mGridColumn;
  String mName;
  UIHorizontalAlignment mHorizontalAlignment = UIHorizontalAlignment::Fill;
  UIVerticalAlignment mVerticalAlignment = UIVerticalAlignment::Fill;

  bool mMouseOver;

  void DrawRect(uint8* buffer, size_t bufferWidth, size_t rWidth, size_t rHeight, const ZColor& color);
};

}
