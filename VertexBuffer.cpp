#include "VertexBuffer.h"

#include <cstring>
#include <cmath>

#include "ZAssert.h"
#include "CommonMath.h"
#include "Constants.h"
#include "PlatformMemory.h"
#include "PlatformIntrinsics.h"
#include "ScopedTimer.h"

static constexpr ZSharp::int32 MAX_INDICIES_AFTER_CLIP = 12;

namespace ZSharp {

VertexBuffer::~VertexBuffer() {
  if (mData != nullptr) {
    PlatformAlignedFree(mData);
  }
}

VertexBuffer::VertexBuffer(const VertexBuffer& rhs) {
  *this = rhs;
}

void VertexBuffer::operator=(const VertexBuffer& rhs) {
  if (this == &rhs) {
    return;
  }

  if ((rhs.mData == nullptr) ||
    (rhs.mAllocatedSize == 0) ||
    (rhs.mInputSize == 0)) {
    return;
  }

  Resize(rhs.mInputSize, rhs.mStride);
  memcpy(mData, rhs.mData, rhs.mAllocatedSize);
}

float* VertexBuffer::operator[](int32 index) const {
  ZAssert((index * mStride) < mAllocatedSize);
  return mData + (index * mStride);
}

float* VertexBuffer::operator[](int32 index) {
  ZAssert((index * mStride) < mAllocatedSize);
  return mData + (index * mStride);
}

int32 VertexBuffer::GetTotalSize() const {
  return mAllocatedSize;
}

int32 VertexBuffer::GetVertSize() const {
  return mWorkingSize / mStride;
}

int32 VertexBuffer::GetStride() const {
  return mStride;
}

void VertexBuffer::CopyInputData(const float* data, int32 index, int32 length) {
  memcpy(mData + index, data, length * sizeof(float));
  mWorkingSize += length;
}

float* VertexBuffer::GetClipData(int32 index) {
  return mClipData + (index * mStride);
}

const float* VertexBuffer::GetClipData(int32 index) const {
  return mClipData + (index * mStride);
}

void VertexBuffer::Resize(int32 vertexSize, int32 stride) {
  if (mData != nullptr) {
    PlatformAlignedFree(mData);
  }

  mInputSize = vertexSize;
  mAllocatedSize = ((vertexSize * sizeof(float)) + (vertexSize * MAX_INDICIES_AFTER_CLIP * sizeof(float)));
  mStride = stride;
  mAllocatedSize = (int32)RoundUpNearestMultiple(mAllocatedSize, 16);
  mData = static_cast<float*>(PlatformAlignedMalloc(mAllocatedSize, 16));
  mClipData = mData + mInputSize;
  mWorkingSize = 0;
  mClipLength = 0;
}

void VertexBuffer::Clear() {
  memset(mData, 0, mAllocatedSize);
  Reset();
}

void VertexBuffer::Reset() {
  mWorkingSize = 0;
  mClipLength = 0;
  mClipData = mData + mInputSize;
}

void VertexBuffer::ApplyTransform(const Mat4x4& transform) {
  Aligned_Mat4x4Transform((const float(*)[4])*transform, mData, mStride, mWorkingSize);
}

void VertexBuffer::AppendClipData(const float* data, int32 lengthBytes, int32 numVertices) {
  int32 usedBytes = (mInputSize + (mClipLength * mStride)) * sizeof(float);
  if ((usedBytes + lengthBytes) > mAllocatedSize) {
    return;
  }

  memcpy(mClipData + (mClipLength * mStride), data, lengthBytes);
  mClipLength += numVertices;
}

int32 VertexBuffer::GetClipLength() const {
  return mClipLength;
}

void VertexBuffer::ShuffleClippedData() {
  const int32 offset = mClipLength * mStride;
  const int32 totalBytes = offset * sizeof(float);
  memmove(mData, mClipData, totalBytes); // Clip data and input data may overlap.
  mClipLength = 0;
  mClipData = mData + offset;
  mWorkingSize = offset;
}

AABB VertexBuffer::ComputeBoundingBox() const {
  NamedScopedTimer(VertexBufferComputeAABB);

  float min[4] = { INFINITY, INFINITY, INFINITY, INFINITY };
  float max[4] = { -INFINITY, -INFINITY, -INFINITY, -INFINITY };

  const int32 stride = mStride;
  const float* vertices = mData;
  const int32 numVertices = mWorkingSize;

  Unaligned_AABB(vertices, numVertices, stride, min, max);

  AABB aabb(min, max);

  return aabb;
}

}
