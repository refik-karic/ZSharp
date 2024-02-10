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

void UILinearPanel::Draw(uint8* screen, size_t width, size_t height, size_t offset) {
  size_t currentOffset = offset;

  switch (mHorizontalAlignment) {
    case UIHorizontalAlignment::Fill:
      break;
    case UIHorizontalAlignment::Left:
      break;
    case UIHorizontalAlignment::Center:
      currentOffset += (mWidth / 2);
      break;
    case UIHorizontalAlignment::Right:
    {
      currentOffset += mWidth;

      if (mFlow == UILinearFlow::Horizontal) {
        for (UIBase* item : mItems) {
          currentOffset -= item->Width();
        }
      }
      else {
        size_t maxWidth = 0;

        for (UIBase* item : mItems) {
          maxWidth = Max(maxWidth, item->Width());
        }

        currentOffset -= maxWidth;
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
      currentOffset += ((mHeight / 2) * mWidth);
      break;
    case UIVerticalAlignment::Bottom:
    {
      currentOffset += (mWidth * mHeight);

      if (mFlow == UILinearFlow::Horizontal) {
        size_t maxHeight = 0;

        for (UIBase* item : mItems) {
          maxHeight = Max(maxHeight, item->Height());
        }

        currentOffset -= (mWidth * maxHeight);
      }
      else {
        for (UIBase* item : mItems) {
          currentOffset -= (mWidth * item->Height());
        }
      }
    }
      break;
  }
  
  // TODO: We need to handle clipping items outside of the frame/container.

  if (mFlow == UILinearFlow::Horizontal) {
    for (UIBase* item : mItems) {
      item->Draw(screen, width, height, currentOffset);

      size_t itemWidth = item->Width();
      itemWidth = Clamp(itemWidth, (size_t)0, mWidth);
      currentOffset += itemWidth;
    }
  }
  else {
    for (UIBase* item : mItems) {
      item->Draw(screen, width, height, currentOffset);

      size_t itemHeight = item->Height();
      itemHeight = Clamp(itemHeight, (size_t)0, mHeight);
      currentOffset += (itemHeight * width);
    }
  }
}

void UILinearPanel::HitTest(int32 x, int32 y) {
  // TODO: Need a way to pass through the screen offset in order to correctly perform the hit test.

  for (UIBase* item : mItems) {
    item->HitTest(x, y);
  }
}

void UILinearPanel::AddItem(UIBase* item) {
  if (!mItems.Contains(item)) {
    mItems.Add(item);
  }
}

}
