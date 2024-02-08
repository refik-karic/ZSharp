#include "UIGrid.h"

namespace ZSharp {

UIGrid::UIGrid(size_t width, size_t height, const String& name) 
  : UIContainer(width, height, name) {
}

void UIGrid::Draw(uint8* screen, size_t width, size_t height, size_t offset) {
  (void)screen;
  (void)width;
  (void)height;
  (void)offset;
}

}
