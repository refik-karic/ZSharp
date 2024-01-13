#pragma once

#include "ZBaseTypes.h"

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

  void CopyInputData(const int32* data, int32 index, int32 length);

  int32* GetInputData();

  void Resize(int32 size);

  void Clear();

  void Reset();

  void RemoveTriangle(int32 index);

  void AppendClipData(const int32* data, const int32 length);

  int32 GetClipLength() const;

  const int32* GetClipData(int32 index) const;

  int32* GetClipData(int32 index);

  void ShuffleClippedData();

  private:
  int32* mData = nullptr;
  int32* mClipData = nullptr;
  int32 mInputSize = 0;
  int32 mAllocatedSize = 0;
  int32 mWorkingSize = 0;
  int32 mClipLength = 0;
};

}
