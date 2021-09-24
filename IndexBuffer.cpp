#include "IndexBuffer.h"

#include <malloc.h>
#include <memory.h>

#include "Constants.h"

namespace ZSharp {

static constexpr size_t MAX_INDICIES_AFTER_CLIP = 4;

IndexBuffer::IndexBuffer(size_t size) :
  mInputSize(size),
  mAllocatedSize(size* MAX_INDICIES_AFTER_CLIP * sizeof(size_t))
{
  mData = static_cast<size_t*>(_aligned_malloc(mAllocatedSize, 16));
  mClipData = mData + mInputSize;
}

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

  memcpy(mData, rhs.mData, rhs.mAllocatedSize);
  mInputSize = rhs.mInputSize;
  mClipLength = rhs.mClipLength;
  mClipData = mData + mInputSize;
  mWorkingSize = rhs.mWorkingSize;
}

size_t IndexBuffer::operator[](size_t index) const {
  return mData[index];
}

size_t& IndexBuffer::operator[](size_t index) {
  return mData[index];
}

size_t IndexBuffer::GetWorkingSize() const {
  return mWorkingSize;
}

void IndexBuffer::CopyInputData(const size_t* data, size_t index, size_t length) {
  memcpy(mData + index, data, length * sizeof(size_t));
  mWorkingSize += length;
}

void IndexBuffer::Clear() {
  memset(mData, 0, mAllocatedSize);
  mClipLength = 0;
  mWorkingSize = 0;
  mClipData = mData + mInputSize;
}

void IndexBuffer::RemoveTriangle(size_t index) {
  memcpy(mData + (index * TRI_VERTS), mData + mWorkingSize - TRI_VERTS, TRI_VERTS * sizeof(size_t));
  
  if(mWorkingSize < TRI_VERTS) {
    mWorkingSize = 0;
  }
  else {
    mWorkingSize -= TRI_VERTS;
  }
}

void IndexBuffer::AppendClipData(const Triangle& triangle) {
    if (mWorkingSize + mClipLength + TRI_VERTS > mAllocatedSize) {
        return;
    }

    const size_t* data = reinterpret_cast<const size_t*>(&triangle);
    memcpy(mClipData + mClipLength, data, TRI_VERTS * sizeof(size_t));
    mClipLength += TRI_VERTS;
}

size_t IndexBuffer::GetClipLength() const {
  return mClipLength;
}

size_t IndexBuffer::GetClipData(size_t index) const {
  return *(mClipData + index);
}

}
