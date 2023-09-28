#pragma once

#include "ZBaseTypes.h"

namespace ZSharp {

class DepthBuffer final {
  public:

  DepthBuffer();

  DepthBuffer(const DepthBuffer&) = delete;
  void operator=(const DepthBuffer&) = delete;

  ~DepthBuffer();

  float& operator[](size_t index);

  void Clear();

  float* GetBuffer();

  size_t GetWidth() const;

  size_t GetHeight() const;

  private:
  float* mData = nullptr;
  size_t mWidth = 0;
  size_t mHeight = 0;

  void OnResize(size_t width, size_t height);
};

}
