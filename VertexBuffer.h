#pragma once

#include <string>
#include <vector>

#include "Constants.h"
#include "Mat4x4.h"
#include "Vec4.h"

namespace ZSharp {

template<typename T>
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

  VertexBuffer(const VertexBuffer<T>& rhs) {
    if (this != &rhs) {
      *this = rhs;
    }
  }

  void operator=(const VertexBuffer<T>& rhs) {
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

  T operator[](std::size_t index) const {
    return mData[index];
  }

  T& operator[](std::size_t index) {
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

  void CopyInputData(const T* data, std::size_t index, std::size_t length) {
    static constexpr T wDefault{1};
    T* currentIndex = mData.data() + index;
    for (std::size_t i = 0; i < length; i += mInputStride) {
      for (std::size_t j = 0; j < mInputStride / Constants::TRI_VERTS; j++) {
        std::memcpy(currentIndex, (data + i) + (j * Constants::TRI_VERTS), Constants::TRI_VERTS * sizeof(T));
        currentIndex[3] = wDefault;
        currentIndex += HOMOGENOUS_3D_SPACE;
        mWorkingSize += HOMOGENOUS_3D_SPACE;
      }
    }
  }

  T* GetInputData(std::size_t index = 0, std::size_t stride = 1) {
    return mData.data() + (index * stride);
  }

  const T* GetInputData(std::size_t index = 0, std::size_t stride = 1) const {
    return mData.data() + (index * stride);
  }

  T* GetClipData(std::size_t index = 0, std::size_t stride = 1) {
    return mClipData + (index * stride);
  }

  const T* GetClipData(std::size_t index = 0, std::size_t stride = 1) const {
    return mClipData + (index * stride);
  }

  void Clear() {
    std::memset(mData.data(), 0, mData.size() * sizeof(T));
    mWorkingSize = 0;
    mClipLength = 0;
    mClipData = mData.data() + mInputSize;
  }

  void ApplyTransform(const Mat4x4<T>& transform) {
    for (std::size_t i = 0; i < mWorkingSize; i += mHomogenizedStride) {
      Vec4<T>& vertexVector = *(reinterpret_cast<Vec4<T>*>(mData.data() + i));
      vertexVector[3] = static_cast<T>(1);
      vertexVector = Mat4x4<T>::ApplyTransform(transform, vertexVector);
    }
  }

  void AppendClipData(const T* data, std::size_t length) {
    if(mInputSize + mClipLength + length > mAllocatedSize) {
      return;
    }

    std::memcpy(mClipData + mClipLength, data, length * sizeof(T));
    mClipLength += length;
  }

  std::size_t GetClipLength() const {
    return mClipLength;
  }

  private:
  static constexpr std::size_t HOMOGENOUS_3D_SPACE = 4;

  std::vector<T> mData;
  T* mClipData;
  std::size_t mInputSize = 0;
  std::size_t mAllocatedSize = 0;
  std::size_t mWorkingSize = 0;
  std::size_t mClipLength = 0;
  std::size_t mInputStride = 0;
  std::size_t mHomogenizedStride = 0;
};

}
