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

  // Clip all stored items to the bounds of this container.
  for (UIBase* item : mItems) {
    item->Width() = Clamp(item->Width(), (size_t)0, mWidth);
    item->Height() = Clamp(item->Height(), (size_t)0, mHeight);
  }

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
    }
    break;
  }

  if (mFlow == UILinearFlow::Horizontal) {
    if (mVerticalAlignment == UIVerticalAlignment::Center) {
      size_t yOffset = 0;
      for (UIBase* item : mItems) {
        yOffset += item->Height();
      }

      yOffset = Min(currY, yOffset);
      currY -= (yOffset / 2);
    }
    else if (mVerticalAlignment == UIVerticalAlignment::Bottom) {
      size_t yOffset = 0;
      for (UIBase* item : mItems) {
        yOffset = Max(yOffset, item->Height());
      }

      yOffset = Min(currY, yOffset);
      currY -= yOffset;
    }

    if (mHorizontalAlignment == UIHorizontalAlignment::Center) {
      size_t xOffset = 0;
      for (UIBase* item : mItems) {
        xOffset += item->Width();
      }

      xOffset = Min(currX, xOffset);
      currX -= (xOffset / 2);
    }
    else if (mHorizontalAlignment == UIHorizontalAlignment::Right) {
      size_t xOffset = 0;
      for (UIBase* item : mItems) {
        xOffset += item->Width();
      }

      xOffset = Min(currX, xOffset);
      currX -= xOffset;
    }

    for (UIBase* item : mItems) {
      size_t itemXOffset = currX;
      size_t itemYOffset = currY;

      switch (item->VerticalAlignment()) {
        case UIVerticalAlignment::Fill:
          break;
        case UIVerticalAlignment::Top:
          break;
        case UIVerticalAlignment::Center:
        {
          size_t centerPoint = (mHeight - itemYOffset - item->Height()) / 2;

          itemYOffset += centerPoint;
        }
          break;
        case UIVerticalAlignment::Bottom:
        {
          if (mHorizontalAlignment == UIHorizontalAlignment::Right) {
            itemYOffset = mHeight - item->Height();
          }
        }
        break;
      }

      item->Layout(itemXOffset, itemYOffset);

      size_t itemWidth = item->Width();
      itemWidth = Clamp(itemWidth, (size_t)0, mWidth);
      currX += itemWidth;
    }
  }
  else {
    if (mVerticalAlignment == UIVerticalAlignment::Center) {
      size_t yOffset = 0;
      for (UIBase* item : mItems) {
        yOffset += item->Height();
      }

      yOffset = Min(currY, yOffset);
      currY -= (yOffset / 2);
    }
    else if (mVerticalAlignment == UIVerticalAlignment::Bottom) {
      size_t yOffset = 0;
      for (UIBase* item : mItems) {
        yOffset += item->Height();
      }

      yOffset = Min(currY, yOffset);
      currY -= yOffset;
    }

    if (mHorizontalAlignment == UIHorizontalAlignment::Center) {
      size_t xOffset = 0;
      for (UIBase* item : mItems) {
        xOffset = Max(xOffset, item->Width());
      }

      xOffset = Min(currX, xOffset);
      currX -= (xOffset / 2);
    }
    else if (mHorizontalAlignment == UIHorizontalAlignment::Right) {
      size_t xOffset = 0;
      for (UIBase* item : mItems) {
        xOffset = Max(xOffset, item->Width());
      }

      xOffset = Min(currX, xOffset);
      currX -= xOffset;
    }

    for (UIBase* item : mItems) {
      size_t itemXOffset = currX;
      size_t itemYOffset = currY;

      switch (item->HorizontalAlignment()) {
        case UIHorizontalAlignment::Fill:
          break;
        case UIHorizontalAlignment::Left:
          break;
        case UIHorizontalAlignment::Center:
        {
          size_t largestItem = 0;
          for (UIBase* currentItem : mItems) {
            largestItem = Max(largestItem, currentItem->Width());
          }

          itemXOffset = (itemXOffset + (largestItem / 2)) - (item->Width() / 2);
        }
          break;
        case UIHorizontalAlignment::Right:
        {
          size_t largestItem = 0;
          for (UIBase* currentItem : mItems) {
            largestItem = Max(largestItem, currentItem->Width());
          }

          itemXOffset = (itemXOffset + largestItem - item->Width());
        }
          break;
      }

      item->Layout(itemXOffset, itemYOffset);

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
