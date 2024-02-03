#include "UILinearPanel.h"

namespace ZSharp {

UILinearPanel::UILinearPanel(size_t width, size_t height, const String& name, UILinearFlow flow)
  : UIContainer(width, height, name), mFlow(flow) {
}

void UILinearPanel::Draw(uint8* screen, size_t width, size_t height) {
  (void)screen;
  (void)width;
  (void)height;
}

}
