#include "GlobalEdgeTable.h"

#include "ZAssert.h"
#include "CommonMath.h"

#include <cstring>

#define DEBUG_TEXTURE 1

namespace ZSharp {
GlobalEdgeTable::GlobalEdgeTable(size_t height, size_t attributeStride)
  : mScanHeight(height), mAttributeStride(attributeStride) {

  mScanLines = (ScanLine*)PlatformMalloc(sizeof(ScanLine) * height);
  memset(mScanLines, 0, sizeof(ScanLine) * height);
  for (size_t i = 0; i < height; ++i) {
    ScanLine* scanLine = mScanLines + i;
    scanLine->x1 = -1;
    scanLine->x2 = -1;
    scanLine->x1Attributes = (float*)PlatformMalloc(sizeof(float) * attributeStride);
    scanLine->x2Attributes = (float*)PlatformMalloc(sizeof(float) * attributeStride);
  }

  mScratchBuffer = (float*)PlatformMalloc(sizeof(float) * attributeStride);
  memset(mScratchBuffer, 0, sizeof(float) * mAttributeStride);
}

GlobalEdgeTable::~GlobalEdgeTable() {
  if (mScanLines != nullptr) {
    for (size_t i = 0; i < mScanHeight; ++i) {
      ScanLine* scanLine = mScanLines + i;
      if (scanLine->x1Attributes != nullptr) {
        PlatformFree(scanLine->x1Attributes);
      }

      if (scanLine->x2Attributes != nullptr) {
        PlatformFree(scanLine->x2Attributes);
      }
    }

    PlatformFree(mScanLines);
  }
}

void GlobalEdgeTable::AddPoint(int32 yIndex, int32 x, const float* attribute) {
  if (yIndex >= mScanHeight) {
    return;
  }

  ScanLine* scanLine = mScanLines + yIndex;

  if ((scanLine->x1 == -1) && (scanLine->x2 == -1)) {
    scanLine->x1 = x;
    scanLine->x2 = x;
    memcpy(scanLine->x1Attributes, attribute, sizeof(float) * mAttributeStride);
    memcpy(scanLine->x2Attributes, attribute, sizeof(float) * mAttributeStride);
  }
  else {
    if(scanLine->x1 > x) {
      scanLine->x1 = x;
      memcpy(scanLine->x1Attributes, attribute, sizeof(float) * mAttributeStride);
    }
    else if(scanLine->x2 < x) {
      scanLine->x2 = x;
      memcpy(scanLine->x2Attributes, attribute, sizeof(float) * mAttributeStride);
    }
  }
}

size_t GlobalEdgeTable::TableHeight() const {
  return mScanHeight;
}

size_t GlobalEdgeTable::AttributeSize() const {
  return mAttributeStride;
}

float* GlobalEdgeTable::AttributeScratchBuffer() {
  return mScratchBuffer;
}

void GlobalEdgeTable::Resize(size_t height, size_t stride) {
  if (mScanLines != nullptr) {
    for (size_t i = 0; i < mScanHeight; ++i) {
      ScanLine* scanLine = mScanLines + i;
      if (scanLine->x1Attributes != nullptr) {
        PlatformFree(scanLine->x1Attributes);
      }

      if (scanLine->x2Attributes != nullptr) {
        PlatformFree(scanLine->x2Attributes);
      }
    }

    PlatformFree(mScanLines);
  }

  mAttributeStride = stride;
  mScanHeight = height;
  mScanLines = (ScanLine*)PlatformMalloc(sizeof(ScanLine) * height);
  for (size_t i = 0; i < height; ++i) {
    ScanLine* scanLine = mScanLines + i;
    memset(scanLine, 0, sizeof(ScanLine));
    scanLine->x1 = -1;
    scanLine->x2 = -1;
    scanLine->x1Attributes = (float*)PlatformMalloc(sizeof(float) * stride);
    scanLine->x2Attributes = (float*)PlatformMalloc(sizeof(float) * stride);
  }
}

void GlobalEdgeTable::Draw(Framebuffer& frameBuffer, const ShadingModeOrder& order, const Texture* texture, size_t startY, size_t endY) {
  const size_t MaxWidth = frameBuffer.GetWidth() - 1;
  
  for (size_t y = startY; y < endY; ++y) {
    ScanLine* scanLine = mScanLines + y;

    if ((scanLine->x1 < 0 && scanLine->x2 < 0) ||
      (scanLine->x1 > MaxWidth && scanLine->x2 > MaxWidth)) {
      // Skip over scan lines that are offscreen.
      ResetScanLine(scanLine);
      continue;
    }

    if (scanLine->x1 == scanLine->x2) {
      float* attributeData = scanLine->x1Attributes;
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

      frameBuffer.SetPixel(scanLine->x1, y, pixel);
    }
    else {
      for (int32 i = scanLine->x1; i < scanLine->x2; ++i) {
#if 1
        float* attributeDataX1 = scanLine->x1Attributes;
        float* attributeDataX2 = scanLine->x2Attributes;

        float xT = ParametricSolveForT(static_cast<float>(i),
          static_cast<float>(scanLine->x1),
          static_cast<float>(scanLine->x2));

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

    ResetScanLine(scanLine);
  }
}

void GlobalEdgeTable::ResetScanLine(ScanLine* scanLine) {
  scanLine->x1 = -1;
  scanLine->x2 = -1;
  memset(scanLine->x1Attributes, 0, sizeof(float) * mAttributeStride);
  memset(scanLine->x2Attributes, 0, sizeof(float) * mAttributeStride);
}

}
