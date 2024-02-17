#include "UIBase.h"

namespace ZSharp {

UIBase::UIBase() 
  : mWidth(0), mHeight(0), mName(""), mX(0), mY(0), mMouseOver(false) {
}

UIBase::UIBase(size_t width, size_t height, const String& name)
  : mWidth(width), mHeight(height), mName(name), mX(0), mY(0), mMouseOver(false) {
}

void UIBase::Layout(size_t x, size_t y) {
  (void)x;
  (void)y;
}

void UIBase::Draw(uint8* screen, size_t width, size_t height) {
  (void)screen;
  (void)width;
  (void)height;
}

bool UIBase::operator==(const UIBase& rhs) const {
  return mName == rhs.mName;
}

size_t& UIBase::Width() {
  return mWidth;
}

size_t& UIBase::Height() {
  return mHeight;
}

UIHorizontalAlignment& UIBase::HorizontalAlignment() {
  return mHorizontalAlignment;
}

UIVerticalAlignment& UIBase::VerticalAlignment() {
  return mVerticalAlignment;
}

void UIBase::SetMouseOver(bool state) {
  mMouseOver = state;
}

void UIBase::DrawRect(uint8* buffer, size_t bufferWidth, size_t rWidth, size_t rHeight, const ZColor& color) {
  uint32* pixelOffset = (uint32*)(buffer + (mY * bufferWidth * 4) + mX * 4);

  uint32 colorValue = color.Color();

  for (size_t y = 0; y < rHeight; ++y) {
    for (size_t x = 0; x < rWidth; ++x) {
      pixelOffset[(y * bufferWidth) + x] = colorValue;
    }
  }

}

void UIBase::HitTest(int32 x, int32 y, bool mouseDown) {
  (void)x;
  (void)y;
  (void)mouseDown;
}

}
