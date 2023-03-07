#include "GlobalEdgeTable.h"

#include "ZAssert.h"
#include "CommonMath.h"

namespace ZSharp {
GlobalEdgeTable::GlobalEdgeTable(size_t height)
  : mEdgeTable(height) {
}

void GlobalEdgeTable::AddPoint(int32 yIndex, int32 x, ZColor color, size_t primitiveIndex) {
  if (yIndex >= mEdgeTable.Size()) {
    //ZAssert(false);
    return;
  }

  ScanLineList& list = mEdgeTable[yIndex];

  if (list.IsEmpty()) {
    ScanLine scanLine(x, x, primitiveIndex, color, color);
    list.PushBack(scanLine);
  }
  else {
    ScanLine& lastScanLine = list[list.Size() - 1];

    if(lastScanLine.primitiveIndex != primitiveIndex) {
      ScanLine scanLine(x, x, primitiveIndex, color, color);
      list.PushBack(scanLine);
    }
    else {
      if(lastScanLine.x1 > x) {
        lastScanLine.x1 = x;
        lastScanLine.x1Color = color;
      }
      else if(lastScanLine.x2 < x) {
        lastScanLine.x2 = x;
        lastScanLine.x2Color = color;
      }
    }
  }
}

void GlobalEdgeTable::Draw(Framebuffer& frameBuffer) {
  for (size_t y = 0; y < mEdgeTable.Size(); ++y) {
    Array<ScanLine>& yList = mEdgeTable[y];
    if (!yList.IsEmpty()) {
      for (ScanLine& line : yList) {

        const size_t MaxWidth = frameBuffer.GetWidth() - 1;
        Clamp(line.x1, 0, (int32)MaxWidth);
        Clamp(line.x2, 0, (int32)MaxWidth);

        if (line.x1 == line.x2) {
          frameBuffer.SetPixel(line.x1, y, line.x1Color);
        }
        else {
          for (size_t i = line.x1; i < line.x2; ++i) {
#if 1
            float xT = ParametricSolveForT(static_cast<float>(i),
              static_cast<float>(line.x1),
              static_cast<float>(line.x2));

            const ZColor color(line.x1Color, line.x2Color, xT);
            frameBuffer.SetPixel(i, y, color.Color());
#else
            ZColor color(line.x2Color.R(), line.x2Color.G(), line.x2Color.B());
            frameBuffer.SetPixel(i, y, color);
#endif
          }
        }
      }
    }
  }
}

}
