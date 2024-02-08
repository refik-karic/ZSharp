#include "UILabel.h"

#include "DebugText.h"

namespace ZSharp {

UILabel::UILabel(size_t width, size_t height, const String& name) 
  : UIElement(width, height, name) {
}

void UILabel::Draw(uint8* screen, size_t width, size_t height, size_t offset) {
  (void)height;

  // TODO: We need to handle clipping/resizing in order to prevent checks like this.
  if ((width * height) < offset) {
    return;
  }

  uint8* currentScreenPos = screen + (offset * 4);

  size_t adjustAmount = ((mWidth / 2) * 4);

  if (adjustAmount < offset) {
    offset -= adjustAmount;
  }

  ZColor color(ZColors::GREEN);

  DrawText(mText, 0, 0, currentScreenPos, width, color);
}

String& UILabel::Text() {
  return mText;
}

}
