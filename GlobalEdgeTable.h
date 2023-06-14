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
  GlobalEdgeTable(const GlobalEdgeTable&) = delete;
  void operator=(const GlobalEdgeTable&) = delete;

  void AddPoint(int32 yIndex, int32 x, size_t primitiveIndex, const float* attribute);
  void Draw(Framebuffer& frameBuffer, const ShadingModeOrder& order, const Texture* texture);

  private:
  
  struct ScanLine {
    int32 x1;
    int32 x2;
    size_t primitiveIndex;
    size_t x1AttributeIndex;
    size_t x2AttributeIndex;

    ScanLine() 
      : x1(0), x2(0), primitiveIndex(0), x1AttributeIndex(0), x2AttributeIndex(0) {

    }

    ScanLine(int32 p1, int32 p2, size_t index, size_t p1AttributeIndex, size_t p2AttributeIndex)
      : x1(p1), x2(p2), primitiveIndex(index), x1AttributeIndex(p1AttributeIndex), x2AttributeIndex(p2AttributeIndex) {

    }
  };

  typedef Array<ScanLine> ScanLineList;

  Array<ScanLineList> mEdgeTable;
  Array<float> mLerpedAttributes;
  size_t mAttributeStride = 0;
  size_t mAttributeIndex = 0;
};

}
