#include "UIButton.h"

#include "PlatformApplication.h"

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
    size_t xOffset = x;
    size_t yOffset = y;

    switch (mLabel->VerticalAlignment()) {
      case UIVerticalAlignment::Fill:
        break;
      case UIVerticalAlignment::Top:
        break;
      case UIVerticalAlignment::Center:
      {
        yOffset += ((mHeight / 2) - (mLabel->Height() / 2));
      }
        break;
      case UIVerticalAlignment::Bottom:
      {
        yOffset += (mHeight - mLabel->Height());
      }
        break;
    }

    switch (mLabel->HorizontalAlignment()) {
      case UIHorizontalAlignment::Fill:
        break;
      case UIHorizontalAlignment::Left:
        break;
      case UIHorizontalAlignment::Center:
      {
        xOffset += ((mWidth / 2) - (mLabel->Width() / 2));
      }
        break;
      case UIHorizontalAlignment::Right:
      {
        xOffset += (mWidth - mLabel->Width());
      }
        break;
    }

    mLabel->Layout(xOffset, yOffset);
  }
}

void UIButton::HitTest(int32 x, int32 y, bool mouseDown) {
  bool isInX = (x >= mX && x <= (mX + mWidth));
  bool isInY = (y >= mY && y <= (mY + mHeight));

  mMouseOver = isInX && isInY;

  if (mMouseOver) {
    AppChangeCursor(AppCursor::Hand);
  }
  else {
    AppChangeCursor(AppCursor::Arrow);
  }

  if (mLabel != nullptr) {
    mLabel->SetMouseOver(mMouseOver);
  }

  if (mMouseOver && mouseDown && OnClickDelegate.IsBound()) {
    AppChangeCursor(AppCursor::Arrow);
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
