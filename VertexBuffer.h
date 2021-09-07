#pragma once

#include <string>
#include <vector>

#include "Constants.h"
#include "Mat4x4.h"

namespace ZSharp {

class VertexBuffer final {
  public:
  VertexBuffer(std::size_t size, std::size_t stride);

  ~VertexBuffer();

  VertexBuffer(const VertexBuffer& rhs);

  void operator=(const VertexBuffer& rhs) {
    if (this == &rhs) {
      return;
    }

    std::memcpy(mData, rhs.mData, rhs.mAllocatedSize);
    mWorkingSize = rhs.mWorkingSize;
    mClipLength = rhs.mClipLength;
    mInputStride = rhs.mInputStride;
    mHomogenizedStride = rhs.mHomogenizedStride;
    mInputSize = rhs.mInputSize;
    mClipData = mData + mInputSize;
  }

  float operator[](std::size_t index) const {
    return mData[index];
  }

  float& operator[](std::size_t index) {
    return mData[index];
  }

  std::size_t GetTotalSize() const;

  std::size_t GetWorkingSize() const;

  std::size_t GetHomogenizedStride() const;

  std::size_t GetInputStride() const;

  void CopyInputData(const float* data, std::size_t index, std::size_t length);

  float* GetInputData(std::size_t index = 0, std::size_t stride = 1);

  const float* GetInputData(std::size_t index = 0, std::size_t stride = 1) const;

  float* GetClipData(std::size_t index = 0, std::size_t stride = 1);

  const float* GetClipData(std::size_t index = 0, std::size_t stride = 1) const;

  void Clear();

  void ApplyTransform(const Mat4x4& transform);

  void AppendClipData(const float* data, std::size_t length);

  std::size_t GetClipLength() const;

  private:
  float* mData;
  float* mClipData;
  std::size_t mInputSize = 0;
  std::size_t mAllocatedSize = 0;
  std::size_t mWorkingSize = 0;
  std::size_t mClipLength = 0;
  std::size_t mInputStride = 0;
  std::size_t mHomogenizedStride = 0;
};

}
