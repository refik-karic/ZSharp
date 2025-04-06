#include "UIContainer.h"

#include "PlatformIntrinsics.h"
#include "ScopedTimer.h"

#include <cstring>

namespace ZSharp {

UIContainer::UIContainer(size_t width, size_t height, const String& name)
  : UIBase(width, height, name), mItemSpacing(0), mBackgroundImage(nullptr) {
}

UIContainer::~UIContainer() {
  if (mBackgroundImage != nullptr) {
    delete mBackgroundImage;
  }

  if (mImageData != nullptr) {
    delete mImageData;
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
  NamedScopedTimer(UIContainerDrawBackgroundImage);

  if (mBackgroundImage == nullptr || mBackgroundImage->GetTexture() == nullptr) {
    return;
  }

  if (mImageWidth != width || mImageHeight != height) {
    if (mImageData != nullptr) {
      mImageData = (uint8*)PlatformReAlloc(mImageData, width * height * 4);
    }
    else {
      mImageData = (uint8*)PlatformMalloc(width * height * 4);
    }

    Texture* texture = mBackgroundImage->GetTexture();

    size_t textureWidth = 0;
    size_t textureHeight = 0;
    uint8* textureData = nullptr;

    for (size_t i = 0; i < texture->NumMips(); ++i) {
      textureWidth = texture->Width(i);
      textureHeight = texture->Height(i);
      textureData = texture->Data(i);

      if (textureWidth < width && textureHeight < height) {
        break;
      }
    }


    BilinearScaleImageImpl(textureData, textureWidth, textureHeight, mImageData, width, height);
    mImageWidth = width;
    mImageHeight = height;
  }

  Aligned_Memcpy(screen, mImageData, width * height * 4);
}

}
