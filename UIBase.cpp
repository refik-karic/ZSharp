#include "UIBase.h"

namespace ZSharp {

UIBase::UIBase() 
  : mWidth(0), mHeight(0), mName("") {
}

UIBase::UIBase(size_t width, size_t height, const String& name)
  : mWidth(width), mHeight(height), mName(name) {
}

void UIBase::Draw(uint8* screen, size_t width, size_t height, size_t offset) {
  (void)screen;
  (void)width;
  (void)height;
  (void)offset;
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

}
