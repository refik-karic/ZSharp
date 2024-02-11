#include "UIFrame.h"

namespace ZSharp {

UIFrame::UIFrame(size_t width, size_t height, UIBase* rootItem)
  : mWidth(width), mHeight(height), mBaseItem(rootItem) {
}

UIFrame::~UIFrame() {
  if (mBaseItem != nullptr) {
    delete mBaseItem;
  }
}

void UIFrame::Layout() {
  mBaseItem->Layout(0, 0);
}

void UIFrame::Draw(uint8* screen, size_t width, size_t height) {
  mBaseItem->Draw(screen, width, height);
}

void UIFrame::OnResize(size_t width, size_t height) {
  mWidth = width;
  mHeight = height;

  if (mBaseItem != nullptr) {
    mBaseItem->Width() = width;
    mBaseItem->Height() = height;
  }
}

void UIFrame::OnMouseMove(int32 x, int32 y) {
  mBaseItem->HitTest(x, y);
}

}
