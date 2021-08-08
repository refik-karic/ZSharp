#pragma once

#include <cstddef>
#include <cstdint>

#include "ZColor.h"

namespace ZSharp {
class Framebuffer final {
  public:
  Framebuffer(std::size_t width, std::size_t height, std::size_t stride);
  ~Framebuffer();

  Framebuffer(const Framebuffer&) = delete;
  void operator=(const Framebuffer&) = delete;

  void SetPixel(const std::size_t x, const std::size_t y, const ZColor color);
  void SetRow(const std::size_t y, const std::size_t x1, const std::size_t x2, const ZColor color);
  void Clear(const ZColor color);
  std::uint8_t* GetBuffer();
  std::size_t GetSize() const;

  private:
  uint8_t* mPixelBuffer = nullptr;
  uint8_t* mScratchBuffer = nullptr;
  std::size_t mWidth;
  std::size_t mHeight;
  std::size_t mStride;
  std::size_t mTotalSize;
};
}
