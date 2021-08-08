#include "Constants.h"
#include "IndexBuffer.h"

namespace ZSharp {

IndexBuffer::IndexBuffer(std::size_t size) :
  mData(size * Constants::MAX_INDICIES_AFTER_CLIP),
  mInputSize(size)
{
  mClipData = mData.data() + mInputSize;
}

IndexBuffer::IndexBuffer(const IndexBuffer& rhs) {
  if (this != &rhs) {
    *this = rhs;
  }
}

void IndexBuffer::operator=(const IndexBuffer& rhs) {
  if (this == &rhs) {
    return;
  }

  mData = rhs.mData;
  mInputSize = rhs.mInputSize;
  mClipLength = rhs.mClipLength;
  mClipData = mData.data() + mInputSize;
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
  std::memcpy(mData.data() + index, data, length * sizeof(std::size_t));
  mWorkingSize += length;
}

void IndexBuffer::Clear() {
  std::memset(mData.data(), 0, mData.size() * sizeof(std::size_t));
  mClipLength = 0;
  mWorkingSize = 0;
  mClipData = mData.data() + mInputSize;
}

void IndexBuffer::RemoveTriangle(std::size_t index) {
  std::memcpy(mData.data() + (index * Constants::TRI_VERTS), mData.data() + mWorkingSize - Constants::TRI_VERTS, Constants::TRI_VERTS * sizeof(std::size_t));
  
  if(mWorkingSize < Constants::TRI_VERTS) {
    mWorkingSize = 0;
  }
  else {
    mWorkingSize -= Constants::TRI_VERTS;
  }
}

std::size_t IndexBuffer::GetClipLength() const {
  return mClipLength;
}

std::size_t IndexBuffer::GetClipData(std::size_t index) const {
  return *(mClipData + index);
}

}
