#include "UIButton.h"

namespace ZSharp {

UIButton::UIButton(size_t width, size_t height, const String& name) 
  : UIElement(width, height, name) {
}

void UIButton::Draw(uint8* screen, size_t width, size_t height) {
  (void)screen;
  (void)width;
  (void)height;
}

}
