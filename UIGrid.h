#pragma once

#include "ZBaseTypes.h"
#include "UIContainer.h"
#include "List.h"

namespace ZSharp {

class UIGridRow {
  public:

  private:
  List<UIBase> mItems;
};

class UIGridColumn {
  public:

  private:
  List<UIBase> mItems;
};

/*
Grid is a uniform column/row grid for grouping UIElements.
*/
class UIGrid : public UIContainer {
  public:

  UIGrid(size_t width, size_t height, const String& name);

  void Draw(uint8* screen, size_t width, size_t height);

  private:
  List<UIGridRow> mRows;
  List<UIGridColumn> mColumns;
};

}
