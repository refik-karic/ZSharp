#pragma once

#include "ZBaseTypes.h"
#include "UIElement.h"
#include "ZString.h"
#include "ZColor.h"

namespace ZSharp {

class UILabel : public UIElement {
  public:

  UILabel(size_t width, size_t height, const String& name);

  virtual void Layout(size_t x, size_t y) override;

  virtual void HitTest(int32 x, int32 y, bool mouseDown) override;

  virtual void Draw(uint8* screen, size_t width, size_t height) override;

  virtual void SetBorderThickness(size_t size) override;

  const String& GetText() const;

  void SetText(const String& string);

  private:
  String mText;
};

}
