#include "VertexBuffer.h"

#include <cstring>
#include <cmath>

#include "ZAssert.h"
#include "CommonMath.h"
#include "Constants.h"
#include "PlatformMemory.h"
#include "PlatformIntrinsics.h"
#include "ScopedTimer.h"

static constexpr ZSharp::int32 MAX_INDICIES_AFTER_CLIP = 12;

namespace ZSharp {

VertexBuffer::~VertexBuffer() {
  if (mXData != nullptr) {
    PlatformAlignedFree(mXData);
  }

  if (mYData != nullptr) {
    PlatformAlignedFree(mYData);
  }

  if (mZData != nullptr) {
    PlatformAlignedFree(mZData);
  }

  if (mWData != nullptr) {
    PlatformAlignedFree(mWData);
  }

  if (mAttributeData != nullptr) {
    PlatformAlignedFree(mAttributeData);
  }
}

VertexBuffer::VertexBuffer(const VertexBuffer& rhs) {
  *this = rhs;
}

void VertexBuffer::operator=(const VertexBuffer& rhs) {
  if (this == &rhs) {
    return;
  }

  if ((rhs.mAllocatedSize == 0) ||
    (rhs.mInputSize == 0)) {
    return;
  }

  Resize(rhs.mInputSize, rhs.mNumAttributes);
  Aligned_Memcpy(mXData, rhs.mXData, rhs.mAllocatedSize);
  Aligned_Memcpy(mYData, rhs.mYData, rhs.mAllocatedSize);
  Aligned_Memcpy(mZData, rhs.mZData, rhs.mAllocatedSize);
  Aligned_Memcpy(mWData, rhs.mWData, rhs.mAllocatedSize);

  if (rhs.mNumAttributes > 0) {
    Aligned_Memcpy(mAttributeData, rhs.mAttributeData, rhs.mAllocatedSize * rhs.mNumAttributes);
  }
}

float* VertexBuffer::GetInputDataX(int32 index) const {
  ZAssert(index < mAllocatedSize);
  return mXData + index;
}

float* VertexBuffer::GetInputDataX(int32 index) {
  ZAssert(index < mAllocatedSize);
  return mXData + index;
}

float* VertexBuffer::GetInputDataY(int32 index) const {
  ZAssert(index < mAllocatedSize);
  return mYData + index;
}

float* VertexBuffer::GetInputDataY(int32 index) {
  ZAssert(index < mAllocatedSize);
  return mYData + index;
}

float* VertexBuffer::GetInputDataZ(int32 index) const {
  ZAssert(index < mAllocatedSize);
  return mZData + index;
}

float* VertexBuffer::GetInputDataZ(int32 index) {
  ZAssert(index < mAllocatedSize);
  return mZData + index;
}

float* VertexBuffer::GetInputDataW(int32 index) const {
  ZAssert(index < mAllocatedSize);
  return mWData + index;
}

float* VertexBuffer::GetInputDataW(int32 index) {
  ZAssert(index < mAllocatedSize);
  return mWData + index;
}

int32 VertexBuffer::GetTotalSize() const {
  return mAllocatedSize;
}

int32 VertexBuffer::GetVertSize() const {
  return mWorkingSize;
}

int32 VertexBuffer::GetNumAttributes() const {
  return mNumAttributes;
}

void VertexBuffer::CopyInputData(const float** data, int32 index, int32 length, int32 numAttributes) {
  Aligned_Memcpy(mXData + index, data[0], length * sizeof(float));
  Aligned_Memcpy(mYData + index, data[1], length * sizeof(float));
  Aligned_Memcpy(mZData + index, data[2], length * sizeof(float));
  Aligned_Memcpy(mWData + index, data[3], length * sizeof(float));

  for (int32 i = 0; i < numAttributes; ++i) {
    Aligned_Memcpy(mAttributeData + (i * (mAllocatedSize / sizeof(float))) + index, data[4 + i], length * sizeof(float));
  }

  mWorkingSize += length;
}

float* VertexBuffer::GetClipDataX(int32 index) {
  return mXClipData + index;
}

const float* VertexBuffer::GetClipDataX(int32 index) const {
  return mXClipData + index;
}

float* VertexBuffer::GetClipDataY(int32 index) {
  return mYClipData + index;
}

const float* VertexBuffer::GetClipDataY(int32 index) const {
  return mYClipData + index;
}

float* VertexBuffer::GetClipDataZ(int32 index) {
  return mZClipData + index;
}

const float* VertexBuffer::GetClipDataZ(int32 index) const {
  return mZClipData + index;
}

float* VertexBuffer::GetClipDataW(int32 index) {
  return mWClipData + index;
}

const float* VertexBuffer::GetClipDataW(int32 index) const {
  return mWClipData + index;
}

float* VertexBuffer::GetAttributeData(int32 index, int32 attribute) const {
  return mAttributeData + (attribute * (mAllocatedSize / sizeof(float))) + index;
}

float* VertexBuffer::GetAttributeData(int32 index, int32 attribute) {
  return mAttributeData + (attribute * (mAllocatedSize / sizeof(float))) + index;
}

float* VertexBuffer::GetAttributeClipData(int32 index, int32 attribute) const {
  return mAttributeClipData + (attribute * (mAllocatedSize / sizeof(float))) + index;
}

float* VertexBuffer::GetAttributeClipData(int32 index, int32 attribute) {
  return mAttributeClipData + (attribute * (mAllocatedSize / sizeof(float))) + index;
}

void VertexBuffer::Resize(int32 vertexSize, int32 numAttributes) {
  if (mXData != nullptr) {
    PlatformAlignedFree(mXData);
  }

  if (mYData != nullptr) {
    PlatformAlignedFree(mYData);
  }

  if (mZData != nullptr) {
    PlatformAlignedFree(mZData);
  }

  if (mWData != nullptr) {
    PlatformAlignedFree(mWData);
  }

  if (mAttributeData != nullptr) {
    PlatformAlignedFree(mAttributeData);
  }

  mInputSize = vertexSize;
  mAllocatedSize = ((vertexSize * sizeof(float)) + (vertexSize * MAX_INDICIES_AFTER_CLIP * sizeof(float)));
  mNumAttributes = numAttributes;
  mAllocatedSize = (int32)RoundUpNearestMultiple(mAllocatedSize, PlatformAlignmentGranularity());
  mXData = static_cast<float*>(PlatformAlignedMalloc(mAllocatedSize, PlatformAlignmentGranularity()));
  mYData = static_cast<float*>(PlatformAlignedMalloc(mAllocatedSize, PlatformAlignmentGranularity()));
  mZData = static_cast<float*>(PlatformAlignedMalloc(mAllocatedSize, PlatformAlignmentGranularity()));
  mWData = static_cast<float*>(PlatformAlignedMalloc(mAllocatedSize, PlatformAlignmentGranularity()));
  mXClipData = mXData + mInputSize;
  mYClipData = mYData + mInputSize;
  mZClipData = mZData + mInputSize;
  mWClipData = mWData + mInputSize;

  if (mNumAttributes > 0) {
    mAttributeData = (float*)PlatformAlignedMalloc(RoundUpNearestMultiple(mNumAttributes * mAllocatedSize, PlatformAlignmentGranularity()), PlatformAlignmentGranularity());
    mAttributeClipData = mAttributeData + mInputSize;
  }

  mWorkingSize = 0;
  mClipLength = 0;
  mWasClipped = false;
}

void VertexBuffer::Clear() {
  Aligned_Memset(mXData, 0, mAllocatedSize);
  Aligned_Memset(mYData, 0, mAllocatedSize);
  Aligned_Memset(mZData, 0, mAllocatedSize);
  Aligned_Memset(mWData, 0, mAllocatedSize);

  for (int32 i = 0; i < mNumAttributes; ++i) {
    Aligned_Memset(mAttributeData + (i * (mAllocatedSize / sizeof(float))), 0, mAllocatedSize);
  }

  Reset();
}

void VertexBuffer::Reset() {
  mWorkingSize = 0;
  mClipLength = 0;
  mXClipData = mXData + mInputSize;
  mYClipData = mYData + mInputSize;
  mZClipData = mZData + mInputSize;
  mWClipData = mWData + mInputSize;

  for (int32 i = 0; i < mNumAttributes; ++i) {
    mAttributeClipData[i] = mAttributeData[i] + mInputSize;
  }

  mWasClipped = false;
}

void VertexBuffer::ApplyTransform(const Mat4x4& transform) {
  float* xyzw[4] = { mXData, mYData, mZData, mWData };
  Aligned_Mat4x4Transform((const float(*)[4])*transform, xyzw, mWorkingSize);
}

void VertexBuffer::AppendClipDataAOS(const float* data, int32 lengthBytes, int32 numVertices, int32 numAttributes) {
  int32 usedBytes = (mInputSize + mClipLength) * sizeof(float);
  if ((usedBytes + lengthBytes) > mAllocatedSize) {
    return;
  }

  mXClipData[mClipLength] = data[0];
  mYClipData[mClipLength] = data[1];
  mZClipData[mClipLength] = data[2];
  mWClipData[mClipLength] = data[3];
  for (int32 i = 0; i < numAttributes; ++i) {
    mAttributeClipData[(i * (mAllocatedSize / sizeof(float))) + mClipLength] = data[4 + i];
  }
  mClipLength += numVertices;
}

int32 VertexBuffer::GetClipLength() const {
  return mClipLength;
}

void VertexBuffer::ShuffleClippedData() {
  const int32 offset = mClipLength;
  const int32 totalBytes = offset * sizeof(float);

  // Clip data and input data may overlap.
  memmove(mXData, mXClipData, totalBytes);
  memmove(mYData, mYClipData, totalBytes);
  memmove(mZData, mZClipData, totalBytes);
  memmove(mWData, mWClipData, totalBytes);
  mXClipData = mXData + offset;
  mYClipData = mYData + offset;
  mZClipData = mZData + offset;
  mWClipData = mWData + offset;

  for (int32 i = 0; i < mNumAttributes; ++i) {
    memmove(mAttributeData + (i * (mAllocatedSize / sizeof(float))), mAttributeClipData + (i * (mAllocatedSize / sizeof(float))), totalBytes);
  }

  mAttributeClipData = mAttributeData + offset;

  mClipLength = 0;
  mWorkingSize = offset;
}

AABB VertexBuffer::ComputeBoundingBox() const {
  NamedScopedTimer(VertexBufferComputeAABB);

  float min[4] = { INFINITY, INFINITY, INFINITY, INFINITY };
  float max[4] = { -INFINITY, -INFINITY, -INFINITY, -INFINITY };

  const float* vertices[4] = { mXData, mYData, mZData, mWData };
  const int32 numVertices = mWorkingSize;

  Unaligned_AABBSOA(vertices, numVertices, min, max);

  AABB aabb(min, max);

  return aabb;
}

bool& VertexBuffer::WasClipped() {
  return mWasClipped;
}

}
