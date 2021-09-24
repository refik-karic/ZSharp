#include "VertexBuffer.h"

#include <malloc.h>
#include <memory.h>

#include "Vec4.h"

namespace ZSharp {
static constexpr size_t MAX_VERTS_AFTER_CLIP = 2;

VertexBuffer::VertexBuffer(size_t size, size_t stride) :
  mClipLength(0),
  mInputSize(size + (size / TRI_VERTS)),
  mAllocatedSize(((size + (size / TRI_VERTS)) + (size * MAX_VERTS_AFTER_CLIP)) * sizeof(float)),
  mInputStride(stride),
  mHomogenizedStride(stride + (stride / TRI_VERTS)) {
  mData = static_cast<float*>(_aligned_malloc(mAllocatedSize, 16));
  mClipData = mData + mInputSize;
}

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

size_t VertexBuffer::GetWorkingSize() const {
  return mWorkingSize;
}

size_t VertexBuffer::GetHomogenizedStride() const {
  return mHomogenizedStride;
}

size_t VertexBuffer::GetInputStride() const {
  return mInputStride;
}

void VertexBuffer::CopyInputData(const float* data, size_t index, size_t length) {
  float* currentIndex = mData + index;
  for (size_t i = 0; i < length; i += mInputStride) {
    for (size_t j = 0; j < mInputStride / TRI_VERTS; j++) {
      memcpy(currentIndex, (data + i) + (j * TRI_VERTS), TRI_VERTS * sizeof(float));
      currentIndex[3] = 1.f;
      currentIndex += 4;
      mWorkingSize += 4;
    }
  }
}

float* VertexBuffer::GetInputData(size_t index, size_t stride) {
  return mData + (index * stride);
}

const float* VertexBuffer::GetInputData(size_t index, size_t stride) const {
  return mData + (index * stride);
}

float* VertexBuffer::GetClipData(size_t index, size_t stride) {
  return mClipData + (index * stride);
}

const float* VertexBuffer::GetClipData(size_t index, size_t stride) const {
  return mClipData + (index * stride);
}

void VertexBuffer::Clear() {
  memset(mData, 0, mAllocatedSize);
  mWorkingSize = 0;
  mClipLength = 0;
  mClipData = mData + mInputSize;
}

void VertexBuffer::ApplyTransform(const Mat4x4& transform) {
  for (size_t i = 0; i < mWorkingSize; i += mHomogenizedStride) {
    Vec4& vertexVector = *(reinterpret_cast<Vec4*>(mData + i));
    vertexVector[3] = 1.f;
    vertexVector = transform.ApplyTransform(vertexVector);
  }
}

void VertexBuffer::AppendClipData(const float* data, size_t length) {
  if (mInputSize + mClipLength + (length * sizeof(float)) > mAllocatedSize) {
    return;
  }

  memcpy(mClipData + mClipLength, data, length * sizeof(float));
  mClipLength += length;
}

size_t VertexBuffer::GetClipLength() const {
  return mClipLength;
}
}
