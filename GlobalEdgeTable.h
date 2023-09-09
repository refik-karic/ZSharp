#pragma once

#include "ZBaseTypes.h"

#include "Array.h"
#include "Framebuffer.h"
#include "ShadingMode.h"
#include "Texture.h"
#include "ZColor.h"

namespace ZSharp {

class GlobalEdgeTable final {
  public:

  GlobalEdgeTable(size_t height, size_t attributeStride);
  ~GlobalEdgeTable();
  GlobalEdgeTable(const GlobalEdgeTable&) = delete;
  void operator=(const GlobalEdgeTable&) = delete;

  void AddPoint(int32 yIndex, int32 x, const float* attribute);
  
  size_t TableHeight() const;
  
  size_t AttributeSize() const;
  
  float* AttributeScratchBuffer();

  void Resize(size_t height, size_t stride);

  void Draw(Framebuffer& frameBuffer, const ShadingModeOrder& order, const Texture* texture, size_t startY, size_t endY);

  private:
  
  struct ScanLine {
    int32 x1;
    int32 x2;
    float* x1Attributes;
    float* x2Attributes;

    ScanLine() 
      : x1(0), x2(0), x1Attributes(nullptr), x2Attributes(nullptr) {

    }

    ScanLine(int32 p1, int32 p2, float* p1AttributeIndex, float* p2AttributeIndex)
      : x1(p1), x2(p2), x1Attributes(p1AttributeIndex), x2Attributes(p2AttributeIndex) {

    }
  };

  void ResetScanLine(ScanLine* scanLine);

  ScanLine* mScanLines = nullptr;
  float* mScratchBuffer = nullptr;
  size_t mScanHeight = 0;
  size_t mAttributeStride = 0;
};

}
