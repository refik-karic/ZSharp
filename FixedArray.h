#pragma once

#include "ZAssert.h"
#include "ZBaseTypes.h"

namespace ZSharp {

template<typename T, size_t N>
class FixedArray final {
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

  T& operator[](size_t index) {
    ZAssert(index < N);
    return mData[index];
  }

  const T& operator[](size_t index) const {
    ZAssert(index < N);
    return mData[index];
  }

  T* GetData() {
    return mData;
  }

  const T* GetData() const {
    return mData;
  }

  size_t Size() const {
    return N;
  }

  void Clear() {
    for (size_t i = 0; i < N; ++i) {
      mData[i] = T();
    }
  }

  void Fill(const T& value) {
    for (size_t i = 0; i < N; ++i) {
      mData[i] = value;
    }
  }

  Iterator begin() const {
    return Iterator(mData);
  }

  Iterator end() const {
    return Iterator(mData + N);
  }

  private:
  T mData[N];
};

}
