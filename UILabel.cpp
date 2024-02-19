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
  : UIElement(width, height, name), mColor(ZColors::GREEN), mHoverColor(ZColors::RED) {
}

void UILabel::Layout(size_t x, size_t y) {
  mX = x;
  mY = y;

  size_t stringLength = mText.Length();
  SetWidth(stringLength * FontWidth + (mBorderThickness * 2));
  SetHeight(FontHeight + (mBorderThickness * 2));
}

void UILabel::HitTest(int32 x, int32 y, bool mouseDown) {
  (void)mouseDown;

  bool isInX = (x >= mX && x <= (mX + mWidth + (mBorderThickness * 2)));
  bool isInY = (y >= mY && y <= (mY + mHeight));

  mMouseOver = isInX && isInY;
}

void UILabel::Draw(uint8* screen, size_t width, size_t height) {
  (void)height;

  uint8* currentScreenPos = screen + (mY * width * 4) + (mX * 4);

  ZColor& drawColor = mMouseOver ? mHoverColor : mColor;

  if (mBorderThickness > 0) {
    DrawBorder(screen, width);
  }

  DrawText(mText, mBorderThickness, mBorderThickness, currentScreenPos, width, drawColor);
}

void UILabel::SetBorderThickness(size_t size) {
  mBorderThickness = size;

  size_t stringLength = mText.Length();
  SetWidth(stringLength * FontWidth + (mBorderThickness * 2));
  SetHeight(FontHeight + (mBorderThickness * 2));
}

const String& UILabel::GetText() const {
  return mText;
}

void UILabel::SetText(const String& string) {
  mText = string;

  size_t stringLength = mText.Length();

  SetWidth(stringLength * FontWidth + (mBorderThickness * 2));
  SetHeight(FontHeight + (mBorderThickness * 2));
}

void UILabel::SetColor(const ZColor& color) {
  mColor = color;
}

void UILabel::SetHighlightColor(const ZColor& color) {
  mHoverColor = color;
}

}
