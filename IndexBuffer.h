#pragma once

namespace ZSharp {

class IndexBuffer final {
  public:

  IndexBuffer() = default;

  ~IndexBuffer();

  IndexBuffer(const IndexBuffer& rhs);

  void operator=(const IndexBuffer& rhs);

  size_t operator[](size_t index) const;

  size_t& operator[](size_t index);

  size_t GetIndexSize() const;

  void CopyInputData(const size_t* data, size_t index, size_t length);

  size_t* GetInputData();

  void Resize(size_t size);

  void Clear();

  void Reset();

  void RemoveTriangle(size_t index);

  void AppendClipData(const size_t* data, const size_t length);

  size_t GetClipLength() const;

  const size_t* GetClipData(size_t index) const;

  size_t* GetClipData(size_t index);

  void ShuffleClippedData();

  private:
  size_t* mData = nullptr;
  size_t* mClipData = nullptr;
  size_t mInputSize = 0;
  size_t mAllocatedSize = 0;
  size_t mWorkingSize = 0;
  size_t mClipLength = 0;
};

}
