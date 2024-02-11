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

void UIBase::HitTest(int32 x, int32 y) {
  (void)x;
  (void)y;
}

}
