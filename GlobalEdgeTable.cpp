#include "GlobalEdgeTable.h"

#include "ZAssert.h"
#include "CommonMath.h"

#include <cstring>

#define DEBUG_TEXTURE 1

namespace ZSharp {
GlobalEdgeTable::GlobalEdgeTable(size_t height, size_t attributeStride)
  : mEdgeTable(height), mAttributeStride(attributeStride), mLerpedAttributes(4096) {
}

void GlobalEdgeTable::AddPoint(int32 yIndex, int32 x, size_t primitiveIndex, const float* attribute) {
  if (yIndex >= mEdgeTable.Size()) {
    return;
  }

  ScanLineList& list = mEdgeTable[yIndex];

  if (list.IsEmpty()) {
    ScanLine scanLine(x, x, primitiveIndex, mAttributeIndex, mAttributeIndex);
    list.PushBack(scanLine);
  }
  else {
    ScanLine& lastScanLine = list[list.Size() - 1];

    if(lastScanLine.primitiveIndex != primitiveIndex) {
      ScanLine scanLine(x, x, primitiveIndex, mAttributeIndex, mAttributeIndex);
      list.PushBack(scanLine);
    }
    else {
      if(lastScanLine.x1 > x) {
        lastScanLine.x1 = x;
        lastScanLine.x1AttributeIndex = mAttributeIndex;
      }
      else if(lastScanLine.x2 < x) {
        lastScanLine.x2 = x;
        lastScanLine.x2AttributeIndex = mAttributeIndex;
      }
    }
  }

  if (mAttributeStride == 0) {
    return;
  }

  // Copy the lerped attributes to a separate buffer since their size can vary.
  size_t stridedAttributeIndex = mAttributeIndex * mAttributeStride;

  // We must check for size and not capacity in this case.
  // If we copy data after the size but before the capacity, it will get cleared on a resize.
  if (stridedAttributeIndex + mAttributeStride >= mLerpedAttributes.Size()) {
    mLerpedAttributes.Resize((stridedAttributeIndex + mAttributeStride) * 2);
  }

  float* attributeData = mLerpedAttributes.GetData() + stridedAttributeIndex;
  memcpy(attributeData, attribute, mAttributeStride * sizeof(float));

  mAttributeIndex++;
}

void GlobalEdgeTable::Draw(Framebuffer& frameBuffer, const ShadingModeOrder& order, const Texture* texture) {
  for (size_t y = 0; y < mEdgeTable.Size(); ++y) {
    Array<ScanLine>& yList = mEdgeTable[y];
    for (ScanLine& line : yList) {
      const size_t MaxWidth = frameBuffer.GetWidth() - 1;
      Clamp(line.x1, 0, (int32)MaxWidth);
      Clamp(line.x2, 0, (int32)MaxWidth);

      if (line.x1 == line.x2) {
        float* attributeData = mLerpedAttributes.GetData() + (line.x1AttributeIndex * mAttributeStride);
        ZColor pixel;
          
        for (ShadingMode& mode : order) {
          switch (mode.mode) {
            case ShadingModes::RGB:
            {
              ZColor tempColor(attributeData[0], attributeData[1], attributeData[2]);
              pixel = tempColor;
            }
              break;
            case ShadingModes::UV:
            {
#if DEBUG_TEXTURE
              ZColor tempColor(texture->Sample(attributeData[0], attributeData[1]));
              pixel = tempColor;
#endif
            }
              break;
            case ShadingModes::Normals:
              break;
          }

          attributeData += mode.length;
        }

        frameBuffer.SetPixel(line.x1, y, pixel);
      }
      else {
        for (size_t i = line.x1; i < line.x2; ++i) {
#if 1
          float* attributeDataX1 = mLerpedAttributes.GetData() + (line.x1AttributeIndex * mAttributeStride);
          float* attributeDataX2 = mLerpedAttributes.GetData() + (line.x2AttributeIndex * mAttributeStride);

          float xT = ParametricSolveForT(static_cast<float>(i),
            static_cast<float>(line.x1),
            static_cast<float>(line.x2));

          ZColor pixel;
          for (ShadingMode& mode : order) {
            switch (mode.mode) {
              case ShadingModes::RGB:
              {
                float r = Lerp(attributeDataX1[0], attributeDataX2[0], xT);
                float g = Lerp(attributeDataX1[1], attributeDataX2[1], xT);
                float b = Lerp(attributeDataX1[2], attributeDataX2[2], xT);
                ZColor tempColor(r, g, b);
                pixel = tempColor;
              }
              break;
              case ShadingModes::UV:
              {
#if DEBUG_TEXTURE
                float u = Lerp(attributeDataX1[0], attributeDataX2[0], xT);
                float v = Lerp(attributeDataX1[1], attributeDataX2[1], xT);
                ZColor tempColor(texture->Sample(u, v));
                pixel = tempColor;
#endif
              }
                break;
              case ShadingModes::Normals:
                break;
            }

            attributeDataX1 += mode.length;
            attributeDataX2 += mode.length;
          }

          frameBuffer.SetPixel(i, y, pixel.Color());
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
