#pragma once

#include "ZBaseTypes.h"
#include "Mat4x4.h"
#include "AABB.h"

namespace ZSharp {

class VertexBuffer final {
  public:

  VertexBuffer() = default;

  ~VertexBuffer();

  VertexBuffer(const VertexBuffer& rhs);

  void operator=(const VertexBuffer& rhs);

  float* operator[](int32 index) const;

  float* operator[](int32 index);

  const float* GetClipData(int32 index) const;

  float* GetClipData(int32 index);

  int32 GetTotalSize() const;

  int32 GetVertSize() const;

  int32 GetStride() const;

  void CopyInputData(const float* data, int32 index, int32 length);

  void Resize(int32 vertexSize, int32 stride);

  void Clear();

  void Reset();

  void ApplyTransform(const Mat4x4& transform);

  void AppendClipData(const float* data, int32 lengthBytes, int32 numVertices);

  int32 GetClipLength() const;

  void ShuffleClippedData();

  AABB ComputeBoundingBox() const;

  private:
  float* mData = nullptr;
  float* mClipData = nullptr;
  int32 mInputSize = 0;
  int32 mAllocatedSize = 0;
  int32 mWorkingSize = 0;
  int32 mClipLength = 0;
  int32 mStride = 0;
};

}
