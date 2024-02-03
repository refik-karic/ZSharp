#include "UIBase.h"

namespace ZSharp {

UIBase::UIBase() 
  : mWidth(0), mHeight(0), mName("") {
}

UIBase::UIBase(size_t width, size_t height, const String& name)
  : mWidth(width), mHeight(height), mName(name) {
}

void UIBase::Draw(uint8* screen, size_t width, size_t height) {
  (void)screen;
  (void)width;
  (void)height;
}

}
