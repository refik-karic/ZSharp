#pragma once

#include "ZBaseTypes.h"
#include "List.h"
#include "UIBase.h"
#include "UIContainer.h"

namespace ZSharp {

enum class UILinearFlow {
  Horizontal,
  Vertical
};

class UILinearPanel : public UIContainer {
  public:

  UILinearPanel(size_t width, size_t height, const String& name, UILinearFlow flow);

  void Draw(uint8* screen, size_t width, size_t height);

  private:
  UILinearFlow mFlow;
  List<UIBase> mItems;
};

}
