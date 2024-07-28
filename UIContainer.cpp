#include "UIContainer.h"

#include <cstring>

namespace ZSharp {

UIContainer::UIContainer(size_t width, size_t height, const String& name)
  : UIBase(width, height, name), mItemSpacing(0), mBackgroundImage(nullptr) {
}

UIContainer::~UIContainer() {
  if (mBackgroundImage != nullptr) {
    delete mBackgroundImage;
  }
}

void UIContainer::SetBackgroundImage(UIImage* image) {
  if (mBackgroundImage != nullptr) {
    delete mBackgroundImage;
  }

  mBackgroundImage = image;
}

void UIContainer::SetSpacing(size_t space) {
  mItemSpacing = space;
}

void UIContainer::DrawBackgroundImage(uint8* screen, size_t width, size_t height) {
  Texture* texture = mBackgroundImage->GetTexture();
  size_t textureWidth = texture->Width(0);
  size_t textureHeight = texture->Height(0);
  size_t textureStride = textureWidth * 4;
  uint8* textureData = texture->Data(0);

  size_t screenStride = width * 4;
  size_t copyStride = textureStride;

  if (height < textureHeight) {
    textureHeight = height;
  }

  if (width < textureWidth) {
    copyStride = width * 4;
  }

  for (size_t y = 0; y < textureHeight; ++y) {
    uint8* current = textureData + (y * textureStride);
    uint8* currentScreen = screen + (y * screenStride);
    memcpy(currentScreen, current, copyStride);
  }
}

}
