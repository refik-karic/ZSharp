#include "UIGrid.h"

#include "CommonMath.h"

namespace ZSharp {

UIGridRow::UIGridRow(float inHeight, const String& inName)
  : height(inHeight), name(inName) {
}

bool UIGridRow::operator==(const UIGridRow& rhs) const {
  return name == rhs.name;
}

size_t UIGridRow::ToPixels(size_t inHeight) {
  return (size_t)((float)inHeight * height);
}

UIGridColumn::UIGridColumn(float inWidth, const String& inName)
  : width(inWidth), name(inName) {
}

bool UIGridColumn::operator==(const UIGridColumn& rhs) const {
  return name == rhs.name;
}

size_t UIGridColumn::ToPixels(size_t inWidth) {
  return (size_t)((float)inWidth * width);
}

UIGrid::UIGrid(size_t width, size_t height, const String& name) 
  : UIContainer(width, height, name) {
}

UIGrid::~UIGrid() {
  for (UIBase* item : mItems) {
    if (item != nullptr) {
      delete item;
    }
  }
}

void UIGrid::Layout(size_t x, size_t y) {
  mX = x;
  mY = y;

  if (InvalidSetup()) {
    return;
  }

  // Clip all stored items to the bounds of this container.
  for (UIBase* item : mItems) {
    UIGridRow* row = GetItemRow(*item);
    UIGridColumn* column = GetItemColumn(*item);

    if (row == nullptr || column == nullptr) {
      ZAssert(false); // Row/Column index does not exist.
      continue;
    }

    size_t columnWidth = column->ToPixels(mWidth);
    size_t rowHeight = row->ToPixels(mHeight);

    item->Width() = Clamp(item->Width(), (size_t)0, columnWidth);
    item->Height() = Clamp(item->Height(), (size_t)0, rowHeight);
  }

  /* 
    Layout the items according to their row / column assignment.

    We don't support partial/overflow items currently.
    Assuming that row widths are fixed to the current column index.

    for(Column col)
      for(Row row)
        Layout(curX, curY)
  */

  size_t currX = x;
  size_t currY = y;
  size_t colIdx = 0;
  size_t rowIdx = 0;

  for (UIGridColumn& column : mColumns) {
    size_t rowY = currY;
    rowIdx = 0;

    for (UIGridRow& row : mRows) {
      LayoutInner(column, row, colIdx, rowIdx, currX, rowY);
      rowY += row.ToPixels(mHeight);
      ++rowIdx;
    }

    currX += column.ToPixels(mWidth);
    ++colIdx;
  }
}

void UIGrid::Draw(uint8* screen, size_t width, size_t height) {
  if (InvalidSetup()) {
    return;
  }

  for (UIBase* item : mItems) {
    item->Draw(screen, width, height);
  }
}

void UIGrid::HitTest(int32 x, int32 y, bool mouseDown) {
  if (InvalidSetup()) {
    return;
  }

  for (UIBase* item : mItems) {
    item->HitTest(x, y, mouseDown);
  }
}

void UIGrid::AddItem(UIBase* item) {
  if (!mItems.Contains(item)) {
    mItems.Add(item);
  }
}

void UIGrid::AddRow(const UIGridRow& row) {
  if (!mRows.Contains(row)) {
    mRows.Add(row);
  }
}

void UIGrid::AddColumn(const UIGridColumn& column) {
  if (!mColumns.Contains(column)) {
    mColumns.Add(column);
  }
}

bool UIGrid::InvalidSetup() {
  return (mColumns.Size() == 0) || (mRows.Size() == 0);
}

UIGridRow* UIGrid::GetItemRow(const UIBase& item) {
  size_t rowIndex = item.GridRow();
  List<UIGridRow>::Iterator rowIter = mRows.begin();

  for (size_t i = 0; i < mRows.Size(); ++i) {
    if (i == rowIndex) {
      return &(*rowIter);
    }

    ++rowIter;
  }

  return nullptr;
}

UIGridColumn* UIGrid::GetItemColumn(const UIBase& item) {
  size_t columnIndex = item.GridColumn();
  List<UIGridColumn>::Iterator columnIter = mColumns.begin();

  for (size_t i = 0; i < mColumns.Size(); ++i) {
    if (i == columnIndex) {
      return &(*columnIter);
    }

    ++columnIter;
  }

  return nullptr;
}

void UIGrid::LayoutInner(UIGridColumn& column, UIGridRow& row, size_t colIdx, size_t rowIdx, size_t x, size_t y) {
  size_t currX = x;
  size_t currY = y;
  
  (void)column;
  (void)row;

  for (UIBase* item : mItems) {
    if (item->GridColumn() != colIdx || item->GridRow() != rowIdx) {
      continue;
    }

    // TODO: We need to decide what kind of layout restrictions we must place here.
    item->Layout(currX, currY);
    currX += item->Width();
  }
}

}
