#pragma once

#include "Constants.h"
#include "Mat4x4.h"

namespace ZSharp {

class VertexBuffer final {
  public:
  VertexBuffer(size_t size, size_t stride);

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

  float operator[](size_t index) const {
    return mData[index];
  }

  float& operator[](size_t index) {
    return mData[index];
  }

  size_t GetTotalSize() const;

  size_t GetWorkingSize() const;

  size_t GetHomogenizedStride() const;

  size_t GetInputStride() const;

  void CopyInputData(const float* data, size_t index, size_t length);

  float* GetInputData(size_t index = 0, size_t stride = 1);

  const float* GetInputData(size_t index = 0, size_t stride = 1) const;

  float* GetClipData(size_t index = 0, size_t stride = 1);

  const float* GetClipData(size_t index = 0, size_t stride = 1) const;

  void Clear();

  void ApplyTransform(const Mat4x4& transform);

  void AppendClipData(const float* data, size_t length);

  size_t GetClipLength() const;

  private:
  float* mData;
  float* mClipData;
  size_t mInputSize = 0;
  size_t mAllocatedSize = 0;
  size_t mWorkingSize = 0;
  size_t mClipLength = 0;
  size_t mInputStride = 0;
  size_t mHomogenizedStride = 0;
};

}
