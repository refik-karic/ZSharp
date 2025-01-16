#pragma once

#include "ZBaseTypes.h"

#include "ZAssert.h"
#include "PlatformDefines.h"

namespace ZSharp {

class IndexBuffer final {
  public:

  IndexBuffer() = default;

  ~IndexBuffer();

  IndexBuffer(const IndexBuffer& rhs);

  void operator=(const IndexBuffer& rhs);

  int32 operator[](int32 index) const;

  int32& operator[](int32 index);

  int32 GetIndexSize() const;

  void SetIndexSize(int32 size);

  void CopyInputData(const int32* data, int32 index, int32 length);

  int32* GetInputData();

  const int32* GetInputData() const;

  void Resize(int32 size);

  void Clear();

  void Reset();

  FORCE_INLINE void RemoveTriangle(int32 index) {
    ZAssert(index <= mWorkingSize);

    int32* srcAddr = mData + (mWorkingSize - 3);
    int32* destAddr = mData + index;
    ((int64*)destAddr)[0] = ((int64*)srcAddr)[0];
    destAddr[2] = srcAddr[2];

    mWorkingSize -= 3;
    ZAssert(mWorkingSize >= 0);
  }

  void AppendClipData(const int32* data, const int32 length);

  int32 GetClipLength() const;

  const int32* GetClipData(int32 index) const;

  int32* GetClipData(int32 index);

  void ShuffleClippedData();

  bool& WasClipped();

  private:
  int32* mData = nullptr;
  int32* mClipData = nullptr;
  int32 mInputSize = 0;
  int32 mAllocatedSize = 0;
  int32 mWorkingSize = 0;
  int32 mClipLength = 0;
  bool mWasClipped = false;
};

}
