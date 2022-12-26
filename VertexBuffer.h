#pragma once

#include "Mat4x4.h"

namespace ZSharp {

class VertexBuffer final {
  public:

  VertexBuffer() = default;

  ~VertexBuffer();

  VertexBuffer(const VertexBuffer& rhs);

  void operator=(const VertexBuffer& rhs);

  float* operator[](size_t index) const;

  float* operator[](size_t index);

  const float* GetClipData(size_t index) const;

  float* GetClipData(size_t index);

  size_t GetTotalSize() const;

  size_t GetVertSize() const;

  size_t GetStride() const;

  void CopyInputData(const float* data, size_t index, size_t length);

  void Resize(size_t vertexSize, size_t stride);

  void Clear();

  void Reset();

  void ApplyTransform(const Mat4x4& transform);

  void AppendClipData(const float* data, size_t lengthBytes, size_t numVertices);

  size_t GetClipLength() const;

  void ShuffleClippedData();

  private:
  float* mData = nullptr;
  float* mClipData = nullptr;
  size_t mInputSize = 0;
  size_t mAllocatedSize = 0;
  size_t mWorkingSize = 0;
  size_t mClipLength = 0;
  size_t mStride = 0;
};

}
