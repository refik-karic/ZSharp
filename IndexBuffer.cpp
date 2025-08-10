#include "IndexBuffer.h"

#include "PlatformMemory.h"
#include "PlatformIntrinsics.h"

#include <cstring>

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
  Aligned_Memcpy(mData, rhs.mData, rhs.mAllocatedSize);
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

void IndexBuffer::SetIndexSize(int32 size) {
  mWorkingSize = size;
}

void IndexBuffer::CopyInputData(const int32* data, int32 index, int32 length) {
  Aligned_Memcpy(mData + index, data, length * sizeof(int32));
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
  mData = static_cast<int32*>(PlatformAlignedMalloc(mAllocatedSize, PlatformAlignmentGranularity()));
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

void IndexBuffer::AppendClipData(const int32* data, const int32 length) {
  ZAssert(mWorkingSize + mClipLength + length < mAllocatedSize);

  memcpy(mClipData + mClipLength, data, length * sizeof(int32));
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
