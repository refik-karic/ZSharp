#pragma once

#include "ZBaseTypes.h"
#include "UIBase.h"
#include "UIImage.h"

namespace ZSharp {

/*
Container is a base class for things like Grid or LinearPanel.
*/
class UIContainer : public UIBase {
  public:

  UIContainer(size_t width, size_t height, const String& name);

  virtual ~UIContainer();

  void SetBackgroundImage(UIImage* image);

  void SetSpacing(size_t space);

  protected:
  void DrawBackgroundImage(uint8* screen, size_t width, size_t height);

  size_t mItemSpacing;
  UIImage* mBackgroundImage = nullptr;
  uint8* mImageData = nullptr;
  size_t mImageWidth = 0;
  size_t mImageHeight = 0;
};

}
