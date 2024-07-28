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

  virtual ~UILinearPanel();

  virtual void Layout(size_t x, size_t y) override;

  virtual void Draw(uint8* screen, size_t width, size_t height) override;

  virtual void HitTest(int32 x, int32 y, bool mouseDown) override;

  void AddItem(UIBase* item);

  private:
  UILinearFlow mFlow;
  List<UIBase*> mItems;
};

}
