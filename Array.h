#pragma once

#include <cstdlib>
#include <cstring>

namespace ZSharp {

template<typename T>
class Array final {
  public:

  class Iterator {
    public:
    Iterator(T* data) : mPtr(data) {}

    Iterator& operator++() {
      mPtr++;
      return *this;
    }

    bool operator==(const Iterator& rhs) {
      return mPtr == rhs.mPtr;
    }

    bool operator!=(const Iterator& rhs) {
      return mPtr != rhs.mPtr;
    }

    T& operator*() const {
      return *mPtr;
    }

    T* operator->() {
      return mPtr;
    }

    private:
    T* mPtr;
  };

  Array() {

  }

  Array(size_t size) {
    FreshAlloc(size);
  }

  ~Array() {
    if (mData != nullptr) {
      free(mData);
    }
  }

  Array(const Array& rhs) {
    FreshAlloc(rhs.mSize);
    memcpy(mData, rhs.mData, rhs.mSize * sizeof(T));
  }

  Array(const Array&&) = delete;

  void operator=(const Array& rhs) {
    if (this != &rhs && rhs.mSize > 0) {
      Clear();
      Resize(rhs.mSize);
      memcpy(mData, rhs.mData, rhs.mSize * sizeof(T));
    }
  }

  void operator=(const Array&&) = delete;

  T& operator[](size_t index) {
    return mData[index];
  }

  const T& operator[](size_t index) const {
    return mData[index];
  }

  T* GetData() {
    return mData;
  }

  const T* GetData() const {
    return mData;
  }

  void Clear() {
    memset(mData, 0, sizeof(T) * mSize);
  }

  size_t Size() const {
    return mSize;
  }

  bool IsEmpty() const {
    return mSize == 0;
  }

  void Resize(size_t size) {
    if (mData == nullptr) {
      FreshAlloc(size);
    }
    else {
      // TODO: This fails with std::map for some reason.
      realloc(mData, size);
      mSize = size;
    }
  }

  void PushBack(T& data) {
    Resize(mSize + 1);
    mData[mSize - 1] = data;
  }

  Iterator begin() const {
    return Iterator(mData);
  }

  Iterator end() const {
    return Iterator(mData + mSize);
  }

  private:
  T* mData = nullptr;
  size_t mSize = 0;

  void FreshAlloc(size_t size) {
    mData = static_cast<T*>(malloc(sizeof(T) * size));
    mSize = size;
    Clear();
  }
};

}
