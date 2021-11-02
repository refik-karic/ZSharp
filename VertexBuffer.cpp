#include "VertexBuffer.h"

#include <malloc.h>
#include <memory.h>

#include "Vec4.h"

static constexpr size_t MAX_INDICIES_AFTER_CLIP = 4;

namespace ZSharp {

VertexBuffer::~VertexBuffer() {
  if (mData != nullptr) {
    _aligned_free(mData);
  }
}

VertexBuffer::VertexBuffer(const VertexBuffer& rhs) {
  *this = rhs;
}

size_t VertexBuffer::GetTotalSize() const {
  return mAllocatedSize;
}

size_t VertexBuffer::GetVertSize() const {
  return mWorkingSize;
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

void VertexBuffer::Resize(size_t vertexSize, size_t stride, size_t indexSize) {
  if (mData != nullptr) {
    _aligned_free(mData);
  }

  mInputSize = vertexSize;
  mIndexSize = indexSize;
  mAllocatedSize = ((vertexSize * sizeof(float)) + (indexSize * MAX_INDICIES_AFTER_CLIP * sizeof(float)));
  mStride = stride;
  mData = static_cast<float*>(_aligned_malloc(mAllocatedSize, 16));
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
  for (size_t i = 0; i < GetVertSize(); ++i) {
    Vec4& vertexVector = *(reinterpret_cast<Vec4*>(mData + (i * mStride)));
    vertexVector = transform.ApplyTransform(vertexVector);
  }
}

void VertexBuffer::AppendClipData(const float* data, size_t length) {
  size_t usedBytes = (mInputSize + (mClipLength * mStride)) * sizeof(float);
  if (usedBytes + (length * sizeof(Vec4)) > mAllocatedSize) {
    return;
  }

  memcpy(mClipData + (mClipLength * mStride), data, length * sizeof(Vec4));
  mClipLength += length;
}

size_t VertexBuffer::GetClipLength() const {
  return mClipLength;
}
}
