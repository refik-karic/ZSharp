#include "VertexBuffer.h"

#include <malloc.h>
#include <memory.h>

#include "Vec4.h"

namespace ZSharp {
VertexBuffer::VertexBuffer(std::size_t size, std::size_t stride) :
  mClipLength(0),
  mInputSize(size + (size / Constants::TRI_VERTS)),
  mAllocatedSize(((size + (size / Constants::TRI_VERTS)) + (size * Constants::MAX_VERTS_AFTER_CLIP)) * sizeof(float)),
  mInputStride(stride),
  mHomogenizedStride(stride + (stride / Constants::TRI_VERTS)) {
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

std::size_t VertexBuffer::GetTotalSize() const {
  return mAllocatedSize;
}

std::size_t VertexBuffer::GetWorkingSize() const {
  return mWorkingSize;
}

std::size_t VertexBuffer::GetHomogenizedStride() const {
  return mHomogenizedStride;
}

std::size_t VertexBuffer::GetInputStride() const {
  return mInputStride;
}

void VertexBuffer::CopyInputData(const float* data, std::size_t index, std::size_t length) {
  float* currentIndex = mData + index;
  for (std::size_t i = 0; i < length; i += mInputStride) {
    for (std::size_t j = 0; j < mInputStride / Constants::TRI_VERTS; j++) {
      memcpy(currentIndex, (data + i) + (j * Constants::TRI_VERTS), Constants::TRI_VERTS * sizeof(float));
      currentIndex[3] = 1.f;
      currentIndex += 4;
      mWorkingSize += 4;
    }
  }
}

float* VertexBuffer::GetInputData(std::size_t index, std::size_t stride) {
  return mData + (index * stride);
}

const float* VertexBuffer::GetInputData(std::size_t index, std::size_t stride) const {
  return mData + (index * stride);
}

float* VertexBuffer::GetClipData(std::size_t index, std::size_t stride) {
  return mClipData + (index * stride);
}

const float* VertexBuffer::GetClipData(std::size_t index, std::size_t stride) const {
  return mClipData + (index * stride);
}

void VertexBuffer::Clear() {
  memset(mData, 0, mAllocatedSize);
  mWorkingSize = 0;
  mClipLength = 0;
  mClipData = mData + mInputSize;
}

void VertexBuffer::ApplyTransform(const Mat4x4& transform) {
  for (std::size_t i = 0; i < mWorkingSize; i += mHomogenizedStride) {
    Vec4& vertexVector = *(reinterpret_cast<Vec4*>(mData + i));
    vertexVector[3] = 1.f;
    vertexVector = transform.ApplyTransform(vertexVector);
  }
}

void VertexBuffer::AppendClipData(const float* data, std::size_t length) {
  if (mInputSize + mClipLength + (length * sizeof(float)) > mAllocatedSize) {
    return;
  }

  memcpy(mClipData + mClipLength, data, length * sizeof(float));
  mClipLength += length;
}

std::size_t VertexBuffer::GetClipLength() const {
  return mClipLength;
}
}
