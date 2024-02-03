#include "UIFrame.h"

namespace ZSharp {

UIFrame::UIFrame(size_t width, size_t height, UIBase& rootItem)
  : mWidth(width), mHeight(height), mBaseItem(rootItem) {
}

void UIFrame::Draw(uint8* screen, size_t width, size_t height) {
  mBaseItem.Draw(screen, width, height);
}

}
