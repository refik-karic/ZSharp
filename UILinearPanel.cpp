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

  mItems.Clear();
}

void UILinearPanel::Layout(size_t x, size_t y) {
  mX = x;
  mY = y;

  size_t currX = x;
  size_t currY = y;

  // Clip all stored items to the bounds of this container.
  for (UIBase* item : mItems) {
    item->SetWidth(Clamp(item->GetWidth(), (size_t)0, GetWidth()));
    item->SetHeight(Clamp(item->GetHeight(), (size_t)0, GetHeight()));
  }

  switch (mHorizontalAlignment) {
    case UIHorizontalAlignment::Fill:
      break;
    case UIHorizontalAlignment::Left:
      break;
    case UIHorizontalAlignment::Center:
    {
      currX += (GetWidth() / 2);
    }
      break;
    case UIHorizontalAlignment::Right:
    {
      currX += GetWidth();
    }
    break;
  }

  switch (mVerticalAlignment) {
    case UIVerticalAlignment::Fill:
      break;
    case UIVerticalAlignment::Top:
      break;
    case UIVerticalAlignment::Center:
    {
      currY += (GetHeight() / 2);
    }
      break;
    case UIVerticalAlignment::Bottom:
    {
      currY += GetHeight();
    }
    break;
  }

  if (mFlow == UILinearFlow::Horizontal) {
    if (mVerticalAlignment == UIVerticalAlignment::Center) {
      size_t yOffset = 0;
      for (UIBase* item : mItems) {
        yOffset += item->GetHeight();
      }

      yOffset = Min(currY, yOffset);
      currY -= (yOffset / 2);
    }
    else if (mVerticalAlignment == UIVerticalAlignment::Bottom) {
      size_t yOffset = 0;
      for (UIBase* item : mItems) {
        size_t itemHeight = item->GetHeight();
        yOffset = Max(yOffset, itemHeight);
      }

      yOffset = Min(currY, yOffset);
      currY -= yOffset;
    }

    if (mHorizontalAlignment == UIHorizontalAlignment::Center) {
      size_t xOffset = 0;
      size_t spacing = 0;
      for (UIBase* item : mItems) {
        xOffset += item->GetWidth();
        spacing += mItemSpacing;
      }

      if (spacing > 0) {
        spacing -= mItemSpacing;
      }

      xOffset = Clamp(xOffset, (size_t)0, mWidth);
      currX -= (xOffset / 2);
      currX -= (spacing / 2);
    }
    else if (mHorizontalAlignment == UIHorizontalAlignment::Right) {
      size_t xOffset = 0;
      size_t spacing = 0;
      for (UIBase* item : mItems) {
        xOffset += item->GetWidth();
        spacing += mItemSpacing;
      }

      if (spacing > 0) {
        spacing -= mItemSpacing;
      }

      xOffset = Clamp(xOffset, (size_t)0, mWidth);
      currX -= xOffset;
      currX -= spacing;
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
          size_t centerPoint = (GetHeight() - itemYOffset - item->GetHeight()) / 2;

          itemYOffset += centerPoint;
        }
          break;
        case UIVerticalAlignment::Bottom:
        {
          if (mHorizontalAlignment == UIHorizontalAlignment::Right) {
            itemYOffset = GetHeight() - item->GetHeight();
          }
        }
        break;
      }

      item->Layout(itemXOffset, itemYOffset);

      size_t itemWidth = item->GetWidth();
      itemWidth = Clamp(itemWidth, (size_t)0, GetWidth());
      currX += (itemWidth + mItemSpacing);
    }
  }
  else {
    if (mVerticalAlignment == UIVerticalAlignment::Center) {
      size_t yOffset = 0;
      size_t spacing = 0;
      for (UIBase* item : mItems) {
        yOffset += item->GetHeight();
        spacing += mItemSpacing;
      }

      if (spacing > 0) {
        spacing -= mItemSpacing;
      }

      yOffset = Clamp(yOffset, (size_t)0, mHeight);
      currY -= (yOffset / 2);
      currY -= (spacing / 2);
    }
    else if (mVerticalAlignment == UIVerticalAlignment::Bottom) {
      size_t yOffset = 0;
      size_t spacing = 0;
      for (UIBase* item : mItems) {
        yOffset += item->GetHeight();
        spacing += mItemSpacing;
      }

      if (spacing > 0) {
        spacing -= mItemSpacing;
      }

      yOffset = Clamp(yOffset, (size_t)0, mHeight);
      currY -= yOffset;
      currY -= spacing;
    }

    if (mHorizontalAlignment == UIHorizontalAlignment::Center) {
      size_t xOffset = 0;
      for (UIBase* item : mItems) {
        size_t itemWidth = item->GetWidth();
        xOffset = Max(xOffset, itemWidth);
      }

      xOffset = Min(currX, xOffset);
      currX -= (xOffset / 2);
    }
    else if (mHorizontalAlignment == UIHorizontalAlignment::Right) {
      size_t xOffset = 0;
      for (UIBase* item : mItems) {
        size_t itemWidth = item->GetWidth();
        xOffset = Max(xOffset, itemWidth);
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
            size_t itemWidth = currentItem->GetWidth();
            largestItem = Max(largestItem, itemWidth);
          }

          itemXOffset = (itemXOffset + (largestItem / 2)) - (item->GetWidth() / 2);
        }
          break;
        case UIHorizontalAlignment::Right:
        {
          size_t largestItem = 0;
          for (UIBase* currentItem : mItems) {
            size_t itemWidth = currentItem->GetWidth();
            largestItem = Max(largestItem, itemWidth);
          }

          itemXOffset = (itemXOffset + largestItem - item->GetWidth());
        }
          break;
      }

      item->Layout(itemXOffset, itemYOffset);

      size_t itemHeight = item->GetHeight();
      itemHeight = Clamp(itemHeight, (size_t)0, GetHeight());
      currY += (itemHeight + mItemSpacing);
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
