#pragma once

#include "Triangle.h"

namespace ZSharp {

class IndexBuffer final {
  public:

  IndexBuffer(size_t size);

  ~IndexBuffer();

  IndexBuffer(const IndexBuffer& rhs);

  void operator=(const IndexBuffer& rhs);

  size_t operator[](size_t index) const;

  size_t& operator[](size_t index);

  size_t GetWorkingSize() const;

  void CopyInputData(const size_t* data, size_t index, size_t length);

  void Clear();

  void RemoveTriangle(size_t index);

  void AppendClipData(const Triangle& triangle);

  size_t GetClipLength() const;

  size_t GetClipData(size_t index) const;

  private:
  size_t* mData;
  size_t* mClipData;
  size_t mInputSize = 0;
  size_t mAllocatedSize = 0;
  size_t mWorkingSize = 0;
  size_t mClipLength = 0;
};

}
