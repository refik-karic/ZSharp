#include "VertexBuffer.h"

#include <cstring>

#include "ZAssert.h"
#include "CommonMath.h"
#include "Constants.h"
#include "PlatformMemory.h"
#include "PlatformIntrinsics.h"
#include "ScopedTimer.h"

static constexpr size_t MAX_INDICIES_AFTER_CLIP = 12;

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

float* VertexBuffer::operator[](size_t index) const {
  ZAssert((index * mStride) < mAllocatedSize);
  return mData + (index * mStride);
}

float* VertexBuffer::operator[](size_t index) {
  ZAssert((index * mStride) < mAllocatedSize);
  return mData + (index * mStride);
}

size_t VertexBuffer::GetTotalSize() const {
  return mAllocatedSize;
}

size_t VertexBuffer::GetVertSize() const {
  return mWorkingSize / mStride;
}

size_t VertexBuffer::GetStride() const {
  return mStride;
}

void VertexBuffer::CopyInputData(const float* data, size_t index, size_t length) {
  memcpy(mData + index, data, length * sizeof(float));
  mWorkingSize += length;
}

float* VertexBuffer::GetClipData(size_t index) {
  return mClipData + (index * mStride);
}

const float* VertexBuffer::GetClipData(size_t index) const {
  return mClipData + (index * mStride);
}

void VertexBuffer::Resize(size_t vertexSize, size_t stride) {
  if (mData != nullptr) {
    PlatformAlignedFree(mData);
  }

  mInputSize = vertexSize;
  mAllocatedSize = ((vertexSize * sizeof(float)) + (vertexSize * MAX_INDICIES_AFTER_CLIP * sizeof(float)));
  mStride = stride;
  mAllocatedSize = RoundUpNearestMultiple(mAllocatedSize, 16);
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

void VertexBuffer::AppendClipData(const float* data, size_t lengthBytes, size_t numVertices) {
  size_t usedBytes = (mInputSize + (mClipLength * mStride)) * sizeof(float);
  if ((usedBytes + lengthBytes) > mAllocatedSize) {
    return;
  }

  memcpy(mClipData + (mClipLength * mStride), data, lengthBytes);
  mClipLength += numVertices;
}

size_t VertexBuffer::GetClipLength() const {
  return mClipLength;
}

void VertexBuffer::ShuffleClippedData() {
  const size_t offset = mClipLength * mStride;
  const size_t totalBytes = offset * sizeof(float);
  memmove(mData, mClipData, totalBytes); // Clip data and input data may overlap.
  mClipLength = 0;
  mClipData = mData + offset;
  mWorkingSize = offset;
}
}
