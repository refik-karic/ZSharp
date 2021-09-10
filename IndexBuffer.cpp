#include "IndexBuffer.h"

#include <malloc.h>
#include <memory.h>

#include "Constants.h"

namespace ZSharp {

IndexBuffer::IndexBuffer(std::size_t size) :
  mInputSize(size),
  mAllocatedSize(size* Constants::MAX_INDICIES_AFTER_CLIP * sizeof(std::size_t))
{
  mData = static_cast<std::size_t*>(_aligned_malloc(mAllocatedSize, 16));
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

std::size_t IndexBuffer::operator[](std::size_t index) const {
  return mData[index];
}

std::size_t& IndexBuffer::operator[](std::size_t index) {
  return mData[index];
}

std::size_t IndexBuffer::GetWorkingSize() const {
  return mWorkingSize;
}

void IndexBuffer::CopyInputData(const std::size_t* data, std::size_t index, std::size_t length) {
  memcpy(mData + index, data, length * sizeof(std::size_t));
  mWorkingSize += length;
}

void IndexBuffer::Clear() {
  memset(mData, 0, mAllocatedSize);
  mClipLength = 0;
  mWorkingSize = 0;
  mClipData = mData + mInputSize;
}

void IndexBuffer::RemoveTriangle(std::size_t index) {
  memcpy(mData + (index * Constants::TRI_VERTS), mData + mWorkingSize - Constants::TRI_VERTS, Constants::TRI_VERTS * sizeof(std::size_t));
  
  if(mWorkingSize < Constants::TRI_VERTS) {
    mWorkingSize = 0;
  }
  else {
    mWorkingSize -= Constants::TRI_VERTS;
  }
}

void IndexBuffer::AppendClipData(const Triangle& triangle) {
    if (mWorkingSize + mClipLength + Constants::TRI_VERTS > mAllocatedSize) {
        return;
    }

    const std::size_t* data = reinterpret_cast<const std::size_t*>(&triangle);
    memcpy(mClipData + mClipLength, data, Constants::TRI_VERTS * sizeof(std::size_t));
    mClipLength += Constants::TRI_VERTS;
}

std::size_t IndexBuffer::GetClipLength() const {
  return mClipLength;
}

std::size_t IndexBuffer::GetClipData(std::size_t index) const {
  return *(mClipData + index);
}

}
