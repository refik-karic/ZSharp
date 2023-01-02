#pragma once

#include "ZBaseTypes.h"

#include "Array.h"
#include "Framebuffer.h"
#include "ZColor.h"

namespace ZSharp {

class GlobalEdgeTable final {
  public:

  GlobalEdgeTable(size_t height);
  GlobalEdgeTable(const GlobalEdgeTable&) = delete;
  void operator=(const GlobalEdgeTable&) = delete;

  void AddPoint(int32 yIndex, int32 x, ZColor color, size_t primitiveIndex);
  void Draw(Framebuffer& frameBuffer);

  private:
  
  struct ScanLine {
    int32 x1;
    int32 x2;
    size_t primitiveIndex;
    ZColor x1Color;
    ZColor x2Color;

    ScanLine() 
      : x1(0), x2(0), primitiveIndex(0) {

    }

    ScanLine(int32 p1, int32 p2, size_t index, ZColor p1Color, ZColor p2Color)
      : x1(p1), x2(p2), primitiveIndex(index), x1Color(p1Color), x2Color(p2Color) {

    }
  };

  typedef Array<ScanLine> ScanLineList;

  Array<ScanLineList> mEdgeTable;
};

}
