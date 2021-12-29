#pragma once

#include <cstddef>

#include "Array.h"
#include "Framebuffer.h"
#include "ZColor.h"

namespace ZSharp {

class GlobalEdgeTable final {
  public:

  GlobalEdgeTable(size_t height);
  GlobalEdgeTable(const GlobalEdgeTable&) = delete;
  void operator=(const GlobalEdgeTable&) = delete;

  void AddPoint(size_t yIndex, size_t x, ZColor color, size_t primitiveIndex);
  void Draw(Framebuffer& frameBuffer);

  private:
  
  struct ScanLine {
    size_t x1;
    size_t x2;
    size_t primitiveIndex;
    ZColor color;
  };

  typedef Array<ScanLine> ScanLineList;

  Array<ScanLineList> mEdgeTable;
};

}
