#include "UILinearPanel.h"

#include "CommonMath.h"

namespace ZSharp {

UILinearPanel::UILinearPanel(size_t width, size_t height, const String& name, UILinearFlow flow)
  : UIContainer(width, height, name), mFlow(flow) {
}

UILinearPanel::~UILinearPanel() {
  for (UIBase* item : mItems) {
    if (item != nullptr) {
      delete item;
    }
  }
}

void UILinearPanel::Layout(size_t x, size_t y) {
  mX = x;
  mY = y;

  size_t currX = x;
  size_t currY = y;

  switch (mHorizontalAlignment) {
    case UIHorizontalAlignment::Fill:
      break;
    case UIHorizontalAlignment::Left:
      break;
    case UIHorizontalAlignment::Center:
      currX += (mWidth / 2);
      break;
    case UIHorizontalAlignment::Right:
    {
      currX += mWidth;

      if (mFlow == UILinearFlow::Horizontal) {
        for (UIBase* item : mItems) {
          currX -= item->Width();
        }
      }
      else {
        size_t maxWidth = 0;

        for (UIBase* item : mItems) {
          maxWidth = Max(maxWidth, item->Width());
        }

        currX -= maxWidth;
      }
    }
    break;
  }

  switch (mVerticalAlignment) {
    case UIVerticalAlignment::Fill:
      break;
    case UIVerticalAlignment::Top:
      break;
    case UIVerticalAlignment::Center:
      currY += (mHeight / 2);
      break;
    case UIVerticalAlignment::Bottom:
    {
      currY += mHeight;

      if (mFlow == UILinearFlow::Horizontal) {
        size_t maxHeight = 0;

        for (UIBase* item : mItems) {
          maxHeight = Max(maxHeight, item->Height());
        }

        currY -= maxHeight;
      }
      else {
        for (UIBase* item : mItems) {
          currY -= item->Height();
        }
      }
    }
    break;
  }

  // TODO: We need to handle clipping items outside of the frame/container.

  if (mFlow == UILinearFlow::Horizontal) {
    for (UIBase* item : mItems) {
      item->Layout(currX, currY);

      size_t itemWidth = item->Width();
      itemWidth = Clamp(itemWidth, (size_t)0, mWidth);
      currX += itemWidth;
    }
  }
  else {
    for (UIBase* item : mItems) {
      item->Layout(currX, currY);

      size_t itemHeight = item->Height();
      itemHeight = Clamp(itemHeight, (size_t)0, mHeight);
      currY += itemHeight;
    }
  }
}

void UILinearPanel::Draw(uint8* screen, size_t width, size_t height) {
  for (UIBase* item : mItems) {
    item->Draw(screen, width, height);
  }
}

void UILinearPanel::HitTest(int32 x, int32 y, bool mouseDown) {
  for (UIBase* item : mItems) {
    item->HitTest(x, y, mouseDown);
  }
}

void UILinearPanel::AddItem(UIBase* item) {
  if (!mItems.Contains(item)) {
    mItems.Add(item);
  }
}

}
