#pragma once

#include "ZBaseTypes.h"
#include "ZColor.h"

namespace ZSharp {
class Framebuffer final {
  public:
  Framebuffer();
  Framebuffer(const ZColor clearColor);
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

  private:
  uint8* mPixelBuffer = nullptr;
  uint8* mScratchBuffer = nullptr;
  size_t mWidth = 0;
  size_t mHeight = 0;
  size_t mStride = 0;
  size_t mTotalSize = 0;

  void OnResize(size_t width, size_t height);
};
}
