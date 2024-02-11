#include "UIButton.h"

namespace ZSharp {

UIButton::UIButton(size_t width, size_t height, const String& name) 
  : UIElement(width, height, name), mLabel(nullptr), mBackground(ZColors::BLACK), mHoverBackground(ZColors::YELLOW) {
}

UIButton::~UIButton() {
  if (mLabel != nullptr) {
    delete mLabel;
  }
}

void UIButton::Layout(size_t x, size_t y) {
  mX = x;
  mY = y;
  
  if (mLabel != nullptr) {
    // TODO: We may want more control over how labels are aligned within a button.
    //  We can probably do that without having to place a Grid/LinearLayout inside of the button.
    size_t heightOffset = mHeight / 2;
    size_t widthOffset = (mWidth / 2) - (mLabel->Width() / 2);

    mLabel->Layout(x + widthOffset, y + heightOffset);
  }
}

void UIButton::HitTest(int32 x, int32 y, bool mouseDown) {
  bool isInX = (x >= mX && x <= (mX + mWidth));
  bool isInY = (y >= mY && y <= (mY + mHeight));

  mMouseOver = isInX && isInY;

  if (mLabel != nullptr) {
    mLabel->SetMouseOver(mMouseOver);
  }

  if (mMouseOver && mouseDown && OnClickDelegate.IsBound()) {
    OnClickDelegate();
  }
}

void UIButton::Draw(uint8* screen, size_t width, size_t height) {
  (void)height;

  uint32* pixelOffset = (uint32*)(screen + (mY * width * 4) + mX * 4);

  uint32 backgroundColor = mMouseOver ? mHoverBackground.Color() : mBackground.Color();

  for (size_t y = 0; y < mHeight; ++y) {
    for (size_t x = 0; x < mWidth; ++x) {
      pixelOffset[(y * width) + x] = backgroundColor;
    }
  }
  
  if (mLabel != nullptr) {
    mLabel->Draw(screen, width, height);
  }
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
