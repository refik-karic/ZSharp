#pragma once

#include <vector>

#include "Constants.h"
#include "Triangle.h"

namespace ZSharp {

class IndexBuffer final {
  public:

  IndexBuffer(std::size_t size);

  IndexBuffer(const IndexBuffer& rhs);

  void operator=(const IndexBuffer& rhs);

  std::size_t operator[](std::size_t index) const;

  std::size_t& operator[](std::size_t index);

  std::size_t GetWorkingSize() const;

  void CopyInputData(const std::size_t* data, std::size_t index, std::size_t length);

  void Clear();

  void RemoveTriangle(std::size_t index);

  void AppendClipData(const Triangle& triangle);

  std::size_t GetClipLength() const;

  std::size_t GetClipData(std::size_t index) const;

  private:
  std::vector<std::size_t> mData;
  std::size_t* mClipData;
  std::size_t mInputSize = 0;
  std::size_t mWorkingSize = 0;
  std::size_t mClipLength = 0;
};

}
