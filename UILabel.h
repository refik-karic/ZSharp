#pragma once

#include "ZBaseTypes.h"
#include "UIElement.h"
#include "ZString.h"
#include "ZColor.h"

namespace ZSharp {

class UILabel : public UIElement {
  public:

  UILabel(size_t width, size_t height, const String& name);

  virtual void Draw(uint8* screen, size_t width, size_t height, size_t offset) override;

  const String& GetText() const;

  void SetText(const String& string);

  void SetColor(const ZColor& color);

  private:
  String mText;
  ZColor mColor;
};

}
