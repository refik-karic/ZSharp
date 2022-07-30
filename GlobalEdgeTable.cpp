#include "GlobalEdgeTable.h"

namespace ZSharp {
GlobalEdgeTable::GlobalEdgeTable(size_t height)
  : mEdgeTable(height) {
}

void GlobalEdgeTable::AddPoint(size_t yIndex, size_t x, ZColor color, size_t primitiveIndex) {
  if (yIndex >= mEdgeTable.Size()) {
    return;
  }

  ScanLineList& list = mEdgeTable[yIndex];

  if (list.IsEmpty()) {
    ScanLine scanLine{
      x,
      x,
      primitiveIndex,
      color
    };

    list.PushBack(scanLine);
  }
  else {
    ScanLine& lastScanLine = list[list.Size() - 1];

    if(lastScanLine.primitiveIndex != primitiveIndex) {
      ScanLine scanLine{
        x,
        x,
        primitiveIndex,
        color
      };

      list.PushBack(scanLine);
    }
    else {
      if(lastScanLine.x1 > x) {
          lastScanLine.x1 = x;
      }
      else if(lastScanLine.x2 < x) {
          lastScanLine.x2 = x;
      }

      lastScanLine.color = color;
    }
  }
}

void GlobalEdgeTable::Draw(Framebuffer& frameBuffer) {
  for (size_t y = 0; y < mEdgeTable.Size(); ++y) {
    Array<ScanLine>& yList = mEdgeTable[y];
    if (!yList.IsEmpty()) {
      for (ScanLine& line : yList) {
        frameBuffer.SetRow(y, line.x1, line.x2, line.color);
      }
    }
  }
}

}
