#include "UILabel.h"

#include "DebugText.h"

namespace ZSharp {

/*
TODO: We're just using the default 8x8 font here for now.
  If we want to get fancy text rendering we will need to change this.
*/
const size_t FontWidth = 8;
const size_t FontHeight = 8;

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

  ZColor color(ZColors::GREEN);

  DrawText(mText, 0, 0, currentScreenPos, width, color);
}

const String& UILabel::GetText() const {
  return mText;
}

void UILabel::SetText(const String& string) {
  mText = string;
  
  size_t stringLength = string.Length();

  mWidth = stringLength * FontWidth;
  mHeight = FontHeight;
}

}
