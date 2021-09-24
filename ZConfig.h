#pragma once

#include <cstdint>
#include <cstddef>

namespace ZSharp{

class ZConfig final {
  public:

  ZConfig(const ZConfig&) = delete;
  void operator=(const ZConfig&) = delete;

  static ZConfig& GetInstance();

  size_t GetViewportWidth() const;

  size_t GetViewportHeight() const;

  size_t GetViewportStride() const;

  size_t GetBytesPerPixel() const;

  void SetViewportWidth(size_t width);

  void SetViewportHeight(size_t height);

  void SetBytesPerPixel(size_t bytesPerPixel);

  bool SizeChanged(size_t width, size_t height);

  private:

  ZConfig();

  size_t mViewportWidth = 0;
  size_t mViewportHeight = 0;
  size_t mBytesPerPixel = 0;
  size_t mViewportStride = 0;
};
}
