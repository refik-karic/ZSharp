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

  ~UILinearPanel();

  virtual void Draw(uint8* screen, size_t width, size_t height, size_t offset) override;

  virtual void HitTest(int32 x, int32 y) override;

  void AddItem(UIBase* item);

  private:
  UILinearFlow mFlow;
  List<UIBase*> mItems;
};

}
