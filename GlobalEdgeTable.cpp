#include "GlobalEdgeTable.h"

namespace ZSharp {

void GlobalEdgeTable::AddPoint(std::size_t yIndex, std::size_t x, ZColor color, std::size_t primitiveIndex) {
  ScanLineList& list = mEdgeTable[yIndex];

  if (list.empty()) {
    ScanLine scanLine{
      x,
      x,
      primitiveIndex,
      color
    };

    list.push_back(scanLine);
  }
  else {
    ScanLine& lastScanLine = list[list.size() - 1];

    if(lastScanLine.primitiveIndex != primitiveIndex) {
      ScanLine scanLine{
        x,
        x,
        primitiveIndex,
        color
      };

      list.push_back(scanLine);
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
