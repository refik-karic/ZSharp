#pragma once

#include "ZBaseTypes.h"
#include "UIContainer.h"
#include "List.h"

namespace ZSharp {

class UIGridRow {
  public:

  UIGridRow(float inHeight, const String& inName);

  bool operator==(const UIGridRow& rhs) const;

  size_t ToPixels(size_t inHeight);

  float height;
  String name;
};

class UIGridColumn {
  public:

  UIGridColumn(float inWidth, const String& inName);

  bool operator==(const UIGridColumn& rhs) const;

  size_t ToPixels(size_t inWidth);

  float width;
  String name;
};

/*
Grid is a uniform column/row grid for grouping UIElements.
*/
class UIGrid : public UIContainer {
  public:

  UIGrid(size_t width, size_t height, const String& name);

  virtual ~UIGrid();

  virtual void Layout(size_t x, size_t y) override;

  virtual void Draw(uint8* screen, size_t width, size_t height) override;

  virtual void HitTest(int32 x, int32 y, bool mouseDown) override;

  virtual void Resize(size_t width, size_t height) override;

  void AddItem(UIBase* item);

  void AddRow(const UIGridRow& row);

  void AddColumn(const UIGridColumn& column);

  private:
  List<UIBase*> mItems;
  List<UIGridRow> mRows;
  List<UIGridColumn> mColumns;

  bool InvalidSetup();

  UIGridRow* GetItemRow(const UIBase& item);
  UIGridColumn* GetItemColumn(const UIBase& item);

  void LayoutInner(UIGridColumn& column, UIGridRow& row, size_t colIdx, size_t rowIdx, size_t x, size_t y);
};

}
