#include "UIElement.h"

namespace ZSharp {

UIElement::UIElement(size_t width, size_t height, const String& name) 
  : UIBase(width, height, name) {
}

void UIElement::SetBackgroundColor(const ZColor& color) {
  mBackgroundColor = color;
}

void UIElement::SetHighlightColor(const ZColor& color) {
  mHighlightColor = color;
}

}
