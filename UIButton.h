#pragma once

#include "ZBaseTypes.h"
#include "UIElement.h"
#include "UILabel.h"
#include "ZColor.h"
#include "ZString.h"

namespace ZSharp {

class UIButton : public UIElement {
  public:

  UIButton(size_t width, size_t height, const String& name);

  ~UIButton();

  virtual void Layout(size_t x, size_t y) override;

  virtual void HitTest(int32 x, int32 y) override;

  virtual void Draw(uint8* screen, size_t width, size_t height) override;

  void SetColor(const ZColor& color);

  void SetLabel(UILabel* label);

  private:
  UILabel* mLabel;
  ZColor mBackground;
  ZColor mHoverBackground;
};

}
