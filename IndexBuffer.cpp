#include "IndexBuffer.h"

#include <malloc.h>
#include "ZAssert.h"
#include <cstring>

#include "Constants.h"

static constexpr size_t MAX_INDICIES_AFTER_CLIP = 4;

namespace ZSharp {

IndexBuffer::~IndexBuffer() {
  if (mData != nullptr) {
    _aligned_free(mData);
  }
}

IndexBuffer::IndexBuffer(const IndexBuffer& rhs) {
  *this = rhs;
}

void IndexBuffer::operator=(const IndexBuffer& rhs) {
  if (this == &rhs) {
    return;
  }

  Resize(rhs.mInputSize);
  memcpy(mData, rhs.mData, rhs.mAllocatedSize);
}

size_t IndexBuffer::operator[](size_t index) const {
  return mData[index];
}

size_t& IndexBuffer::operator[](size_t index) {
  return mData[index];
}

size_t IndexBuffer::GetIndexSize() const {
  return mWorkingSize;
}

void IndexBuffer::CopyInputData(const size_t* data, size_t index, size_t length) {
  memcpy(mData + index, data, length * sizeof(size_t));
  mWorkingSize += length;
}

void IndexBuffer::Resize(size_t size) {
  if (mData != nullptr) {
    _aligned_free(mData);
  }

  mInputSize = size;
  mAllocatedSize = size * MAX_INDICIES_AFTER_CLIP * sizeof(size_t);
  mData = static_cast<size_t*>(_aligned_malloc(mAllocatedSize, 16));
  mClipData = mData + mInputSize;
  mClipLength = 0;
  mWorkingSize = 0;
}

void IndexBuffer::Clear() {
  memset(mData, 0, mAllocatedSize);
  Reset();
}

void IndexBuffer::Reset() {
  mClipLength = 0;
  mWorkingSize = 0;
  mClipData = mData + mInputSize;
}

void IndexBuffer::RemoveTriangle(size_t index) {
  ZAssert(index <= mWorkingSize);

  size_t* srcAddr = mData + (mWorkingSize - TRI_VERTS);
  size_t* destAddr = mData + index;
  memcpy(destAddr, srcAddr, TRI_VERTS * sizeof(size_t));
  
  if (mWorkingSize < TRI_VERTS) {
    mWorkingSize = 0;
  }
  else {
    mWorkingSize -= TRI_VERTS;
  }

  ZAssert((mWorkingSize % TRI_VERTS) == 0);
}

void IndexBuffer::AppendClipData(const Triangle& triangle) {
  if (mWorkingSize + mClipLength + TRI_VERTS > mAllocatedSize) {
      return;
  }

  memcpy(mClipData + mClipLength, &triangle, sizeof(Triangle));
  mClipLength += TRI_VERTS;
}

size_t IndexBuffer::GetClipLength() const {
  return mClipLength;
}

size_t IndexBuffer::GetClipData(size_t index) const {
  return *(mClipData + index);
}

}
