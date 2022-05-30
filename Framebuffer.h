#pragma once

#include "ZBaseTypes.h"
#include "ZColor.h"

namespace ZSharp {
class Framebuffer final {
  public:
  Framebuffer(size_t width, size_t height, size_t stride);
  ~Framebuffer();

  Framebuffer(const Framebuffer&) = delete;
  void operator=(const Framebuffer&) = delete;

  void SetPixel(const size_t x, const size_t y, const ZColor color);
  void SetRow(const size_t y, const size_t x1, const size_t x2, const ZColor color);
  void Clear(const ZColor color);
  uint8* GetBuffer();
  size_t GetWidth() const;
  size_t GetHeight() const;
  size_t GetSize() const;
  void Resize();

  private:
  uint8* mPixelBuffer = nullptr;
  uint8* mScratchBuffer = nullptr;
  size_t mWidth;
  size_t mHeight;
  size_t mStride;
  size_t mTotalSize;
};
}
