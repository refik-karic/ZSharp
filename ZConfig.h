#pragma once

#include <cstdint>
#include <cstddef>

namespace ZSharp{

class ZConfig final {
  public:

  ZConfig(const ZConfig&) = delete;
  void operator=(const ZConfig&) = delete;

  static ZConfig& GetInstance();

  std::size_t GetViewportWidth() const;

  std::size_t GetViewportHeight() const;

  std::size_t GetViewportStride() const;

  std::size_t GetBytesPerPixel() const;

  void SetViewportWidth(std::size_t width);

  void SetViewportHeight(std::size_t height);

  void SetBytesPerPixel(std::size_t bytesPerPixel);

  private:

  ZConfig();

  std::size_t mViewportWidth = 0;
  std::size_t mViewportHeight = 0;
  std::size_t mBytesPerPixel = 0;
  std::size_t mViewportStride = 0;
};
}
