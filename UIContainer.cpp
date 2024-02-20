#include "UIContainer.h"

namespace ZSharp {

UIContainer::UIContainer(size_t width, size_t height, const String& name)
  : UIBase(width, height, name), mItemSpacing(0) {
}

void UIContainer::SetSpacing(size_t space) {
  mItemSpacing = space;
}

}
