#pragma once

#include "ZBaseTypes.h"
#include "ZAssert.h"

namespace ZSharp {

template<typename T>
class Span final {
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

  Span() = default;

  Span(T* data, size_t size) 
    : mData(data), mSize(size) {

  }

  Span(const Span& rhs) 
    : mData(rhs.mData), mSize(rhs.mSize) {

  }

  void operator=(const Span& rhs) {
    if (&rhs != this) {
      mData = rhs.mData;
      mSize = rhs.mSize;
    }
  }

  T& operator[](size_t index) {
    ZAssert(index < mSize);
    return mData[index];
  }

  const T& operator[](size_t index) const {
    ZAssert(index < mSize);
    return mData[index];
  }

  T* GetData() {
    return mData;
  }

  const T* GetData() const {
    return mData;
  }

  size_t Size() const {
    return mSize;
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
};

}
