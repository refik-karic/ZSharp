#include "UIButton.h"

#include "CommonMath.h"
#include "PlatformApplication.h"

namespace ZSharp {

UIButton::UIButton(size_t width, size_t height, const String& name) 
  : UIElement(width, height, name), mLabel(nullptr) {
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
        yOffset += ((mHeight >> 1) - (mLabel->GetHeight() >> 1));
      }
        break;
      case UIVerticalAlignment::Bottom:
      {
        yOffset += (mHeight - mLabel->GetHeight());
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
        xOffset += ((mWidth >> 1) - (mLabel->GetWidth() >> 1));
      }
        break;
      case UIHorizontalAlignment::Right:
      {
        xOffset += (mWidth - mLabel->GetWidth());
      }
        break;
    }

    mLabel->Layout(xOffset, yOffset);
  }
}

void UIButton::HitTest(int32 x, int32 y, bool mouseDown) {
  bool isInX = (x >= mX && x <= (mX + mWidth + (mBorderThickness * 2)));
  bool isInY = (y >= mY && y <= (mY + mHeight));

  mMouseOver = isInX && isInY;

  if (mMouseOver) {
    GetApplication()->ApplyCursor(AppCursor::Hand);
  }

  if (mLabel != nullptr) {
    mLabel->SetMouseOver(mMouseOver);
  }

  if (mMouseOver && mouseDown && OnClickDelegate.IsBound()) {
    OnClickDelegate();
  }
}

void UIButton::Draw(uint8* screen, size_t width, size_t height) {
  size_t rWidth = Clamp(mWidth, (size_t)0, width - mX);
  size_t rHeight = Clamp(mHeight, (size_t)0, height - mY);
  ZColor& color = mMouseOver ? mHighlightColor : mBackgroundColor;

  if (mBorderThickness > 0) {
    DrawBorder(screen, width);
  }

  DrawRect(screen, width, rWidth, rHeight, color);
  
  if (mLabel != nullptr) {
    mLabel->Draw(screen, width, height);
  }
}

void UIButton::SetLabel(UILabel* label) {
  if (mLabel != nullptr) {
    delete mLabel;
  }

  mLabel = label;
}

}
