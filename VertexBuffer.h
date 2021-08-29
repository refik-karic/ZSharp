#pragma once

#include <string>
#include <vector>

#include "Constants.h"
#include "Mat4x4.h"
#include "Vec4.h"

namespace ZSharp {

class VertexBuffer final {
  public:
  VertexBuffer(std::size_t size, std::size_t stride) :
    mClipLength(0),
    mInputSize(size + (size / Constants::TRI_VERTS)),
    mAllocatedSize((size + (size / Constants::TRI_VERTS)) + (size * Constants::MAX_VERTS_AFTER_CLIP)),
    mData((size + (size / Constants::TRI_VERTS)) + (size * Constants::MAX_VERTS_AFTER_CLIP)),
    mInputStride(stride),
    mHomogenizedStride(stride + (stride / Constants::TRI_VERTS))
  {
    mClipData = mData.data() + mInputSize;
  }

  VertexBuffer(const VertexBuffer& rhs) {
    *this = rhs;
  }

  void operator=(const VertexBuffer& rhs) {
    if (this == &rhs) {
      return;
    }

    mData = rhs.mData;
    mWorkingSize = rhs.mWorkingSize;
    mClipLength = rhs.mClipLength;
    mInputStride = rhs.mInputStride;
    mHomogenizedStride = rhs.mHomogenizedStride;
    mInputSize = rhs.mInputSize;
    mClipData = mData.data() + mInputSize;
  }

  float operator[](std::size_t index) const {
    return mData[index];
  }

  float& operator[](std::size_t index) {
    return mData[index];
  }

  std::size_t GetTotalSize() const {
    return mAllocatedSize;
  }

  std::size_t GetWorkingSize() const {
    return mWorkingSize;
  }

  std::size_t GetHomogenizedStride() const {
    return mHomogenizedStride;
  }

  std::size_t GetInputStride() const {
    return mInputStride;
  }

  void CopyInputData(const float* data, std::size_t index, std::size_t length) {
    float* currentIndex = mData.data() + index;
    for (std::size_t i = 0; i < length; i += mInputStride) {
      for (std::size_t j = 0; j < mInputStride / Constants::TRI_VERTS; j++) {
        std::memcpy(currentIndex, (data + i) + (j * Constants::TRI_VERTS), Constants::TRI_VERTS * sizeof(float));
        currentIndex[3] = 1.f;
        currentIndex += HOMOGENOUS_3D_SPACE;
        mWorkingSize += HOMOGENOUS_3D_SPACE;
      }
    }
  }

  float* GetInputData(std::size_t index = 0, std::size_t stride = 1) {
    return mData.data() + (index * stride);
  }

  const float* GetInputData(std::size_t index = 0, std::size_t stride = 1) const {
    return mData.data() + (index * stride);
  }

  float* GetClipData(std::size_t index = 0, std::size_t stride = 1) {
    return mClipData + (index * stride);
  }

  const float* GetClipData(std::size_t index = 0, std::size_t stride = 1) const {
    return mClipData + (index * stride);
  }

  void Clear() {
    std::memset(mData.data(), 0, mData.size() * sizeof(float));
    mWorkingSize = 0;
    mClipLength = 0;
    mClipData = mData.data() + mInputSize;
  }

  void ApplyTransform(const Mat4x4& transform) {
    for (std::size_t i = 0; i < mWorkingSize; i += mHomogenizedStride) {
      Vec4& vertexVector = *(reinterpret_cast<Vec4*>(mData.data() + i));
      vertexVector[3] = 1.f;
      vertexVector = Mat4x4::ApplyTransform(transform, vertexVector);
    }
  }

  void AppendClipData(const float* data, std::size_t length) {
    if(mInputSize + mClipLength + length > mAllocatedSize) {
      return;
    }

    std::memcpy(mClipData + mClipLength, data, length * sizeof(float));
    mClipLength += length;
  }

  std::size_t GetClipLength() const {
    return mClipLength;
  }

  private:
  static constexpr std::size_t HOMOGENOUS_3D_SPACE = 4;

  std::vector<float> mData;
  float* mClipData;
  std::size_t mInputSize = 0;
  std::size_t mAllocatedSize = 0;
  std::size_t mWorkingSize = 0;
  std::size_t mClipLength = 0;
  std::size_t mInputStride = 0;
  std::size_t mHomogenizedStride = 0;
};

}
