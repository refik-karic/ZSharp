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
      for (UIBase* item : mItems) {
        currentOffset -= item->Width();
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
      for (UIBase* item : mItems) {
        currentOffset -= (mWidth * item->Height());
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

void UILinearPanel::AddItem(UIBase* item) {
  if (!mItems.Contains(item)) {
    mItems.Add(item);
  }
}

}
