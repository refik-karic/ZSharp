#include "UILabel.h"

namespace ZSharp {

UILabel::UILabel(size_t width, size_t height, const String& name) 
  : UIElement(width, height, name) {
}

void UILabel::Draw(uint8* screen, size_t width, size_t height) {
  (void)screen;
  (void)width;
  (void)height;
}

}
