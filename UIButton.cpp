#include "UIButton.h"

namespace ZSharp {

UIButton::UIButton(size_t width, size_t height, const String& name) 
  : UIElement(width, height, name), mLabel(nullptr) {
}

UIButton::~UIButton() {
  if (mLabel != nullptr) {
    delete mLabel;
  }
}

void UIButton::Draw(uint8* screen, size_t width, size_t height, size_t offset) {
  (void)height;

  uint32* pixelOffset = (uint32*)(screen + offset * 4);
  uint32 backgroundColor = mBackground.Color();

  for (size_t y = 0; y < mHeight; ++y) {
    for (size_t x = 0; x < mWidth; ++x) {
      pixelOffset[(y * width) + x] = backgroundColor;
    }
  }

  // TODO: We may want more control over how labels are aligned within a button.
  //  We can probably do that without having to place a Grid/LinearLayout inside of the button.
  size_t heightOffset = mHeight / 2;
  size_t widthOffset = (mWidth / 2) - (mLabel->Width() / 2);
  
  mLabel->Draw(screen, width, height, offset + widthOffset + (heightOffset * width));
}

void UIButton::SetColor(const ZColor& color) {
  mBackground = color;
}

void UIButton::SetLabel(UILabel* label) {
  if (mLabel != nullptr) {
    delete mLabel;
  }

  mLabel = label;
}

}
