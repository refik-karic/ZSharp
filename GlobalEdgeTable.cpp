#include "GlobalEdgeTable.h"

namespace ZSharp {

void GlobalEdgeTable::AddPoint(size_t yIndex, size_t x, ZColor color, size_t primitiveIndex) {
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

void GlobalEdgeTable::Clear() {
  mEdgeTable.clear();
}

void GlobalEdgeTable::Draw(Framebuffer& frameBuffer) {
  for (auto& [y, scanLineList] : mEdgeTable) {
    for (auto& scanLine : scanLineList) {
      frameBuffer.SetRow(y, scanLine.x1, scanLine.x2, scanLine.color);
    }
  }
}

}
