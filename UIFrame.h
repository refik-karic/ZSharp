#pragma once

#include "ZBaseTypes.h"
#include "UIBase.h"

namespace ZSharp {

/*
Flow should go something like this:
  1. Create UIFrame with a base item (i.e. Grid/Label/Button/etc)
  2. Populate the base item accordingly
  3. When it's time to render, draw from the root down the tree
  4. Each item should override Draw() to know where and how to draw itself
    4a. i.e. Grid knows it's column/row layout and can arrange those items in the correct layout before calling Draw() on them
*/

/*
UIFrame is the root of a given UI screen.
The frame fills a viewport and stores all of its child elements either directly or via Grids/Panels.
*/
class UIFrame {
  public:

  UIFrame(size_t width, size_t height, UIBase* rootItem);

  ~UIFrame();

  void Draw(uint8* screen, size_t width, size_t height);

  void OnResize(size_t width, size_t height);

  private:
  size_t mWidth;
  size_t mHeight;
  UIBase* mBaseItem;
};

}
