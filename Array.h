#pragma once

#include <cstdlib>

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

    Iterator operator++(int) {
      Iterator temp(*this);
      ++(*this);
      return temp;
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
      delete[] mData;
      mData = nullptr;
    }
  }

  Array(const Array& rhs) {
    FreshAlloc(rhs.mSize);
    for (size_t i = 0; i < rhs.mSize; ++i) {
      mData[i] = rhs[i];
    }
  }

  Array(const Array&&) = delete;

  void operator=(const Array& rhs) {
    if (this != &rhs && rhs.mSize > 0) {
      Resize(rhs.mSize);
      for (size_t i = 0; i < rhs.mSize; ++i) {
        mData[i] = rhs[i];
      }
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
    for (size_t i = 0; i < mSize; ++i) {
      mData[i] = T();
    }
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
    else if (mSize != size) {
      T* newAlloc = new T[size];
      size_t copyLength = (size > mSize) ? mSize : size;

      for (size_t i = 0; i < copyLength; ++i) {
        newAlloc[i] = mData[i];
      }

      delete[] mData;
      mData = newAlloc;
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
    mData = new T[size]();
    mSize = size;
  }
};

}
