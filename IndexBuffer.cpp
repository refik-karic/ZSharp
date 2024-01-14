#include "IndexBuffer.h"

#include "PlatformMemory.h"
#include "ZAssert.h"
#include <cstring>

#include "Constants.h"

static constexpr ZSharp::int32 MAX_INDICIES_AFTER_CLIP = 4;

namespace ZSharp {

IndexBuffer::~IndexBuffer() {
  if (mData != nullptr) {
    PlatformAlignedFree(mData);
  }
}

IndexBuffer::IndexBuffer(const IndexBuffer& rhs) {
  *this = rhs;
}

void IndexBuffer::operator=(const IndexBuffer& rhs) {
  if (this == &rhs) {
    return;
  }

  if ((rhs.mData == nullptr) ||
    (rhs.mAllocatedSize == 0)) {
    return;
  }

  Resize(rhs.mInputSize);
  memcpy(mData, rhs.mData, rhs.mAllocatedSize);
}

int32 IndexBuffer::operator[](int32 index) const {
  return mData[index];
}

int32& IndexBuffer::operator[](int32 index) {
  return mData[index];
}

int32 IndexBuffer::GetIndexSize() const {
  return mWorkingSize;
}

void IndexBuffer::CopyInputData(const int32* data, int32 index, int32 length) {
  memcpy(mData + index, data, length * sizeof(int32));
  mWorkingSize += length;
}

int32* IndexBuffer::GetInputData() {
  return mData;
}

const int32* IndexBuffer::GetInputData() const {
  return mData;
}

void IndexBuffer::Resize(int32 size) {
  if (mData != nullptr) {
    PlatformAlignedFree(mData);
  }

  mInputSize = size;
  mAllocatedSize = size * MAX_INDICIES_AFTER_CLIP * sizeof(int32);
  mData = static_cast<int32*>(PlatformAlignedMalloc(mAllocatedSize, 16));
  mClipData = mData + mInputSize;
  mClipLength = 0;
  mWorkingSize = 0;
  mWasClipped = false;
}

void IndexBuffer::Clear() {
  memset(mData, 0, mAllocatedSize);
  Reset();
}

void IndexBuffer::Reset() {
  mClipLength = 0;
  mWorkingSize = 0;
  mClipData = mData + mInputSize;
  mWasClipped = false;
}

void IndexBuffer::RemoveTriangle(int32 index) {
  ZAssert(index <= mWorkingSize);

  int32* srcAddr = mData + (mWorkingSize - TRI_VERTS);
  int32* destAddr = mData + index;
  memcpy(destAddr, srcAddr, TRI_VERTS * sizeof(int32));
  
  if (mWorkingSize < TRI_VERTS) {
    mWorkingSize = 0;
  }
  else {
    mWorkingSize -= TRI_VERTS;
  }

  ZAssert((mWorkingSize % TRI_VERTS) == 0);
}

void IndexBuffer::AppendClipData(const int32* data, const int32 length) {
  if (mWorkingSize + mClipLength + length > mAllocatedSize) {
    return;
  }

  memcpy(mClipData + mClipLength, data, length * sizeof(size_t));
  mClipLength += length;
}

int32 IndexBuffer::GetClipLength() const {
  return mClipLength;
}

const int32* IndexBuffer::GetClipData(int32 index) const {
  return mClipData + index;
}

int32* IndexBuffer::GetClipData(int32 index) {
  return mClipData + index;
}

void IndexBuffer::ShuffleClippedData() {
  const int32 offset = mClipLength;
  const int32 totalBytes = offset * sizeof(int32);
  memmove(mData, mClipData, totalBytes); // Clip data and input data may overlap.
  mClipLength = 0;
  mClipData = mData + offset;
  mWorkingSize = offset;
}

bool& IndexBuffer::WasClipped() {
  return mWasClipped;
}

}
