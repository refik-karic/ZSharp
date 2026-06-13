#pragma once

#include "ZBaseTypes.h"
#include "ZAssert.h"

#include <type_traits>

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

    Iterator& operator--() {
      mPtr--;
      return *this;
    }

    Iterator operator--(int) {
      Iterator temp(mPtr);
      --(*this);
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

  Span(Span&&) = delete;

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

  bool operator==(const Span& rhs) const {
    if (mSize != rhs.mSize) {
      return false;
    }

    if constexpr (std::is_trivially_copyable_v<T> && std::has_unique_object_representations_v<T>) {
      return memcmp(mData, rhs.mData, mSize * sizeof(T)) == 0;
    }
    else {
      for (size_t i = 0; i < mSize; ++i) {
        if (mData[i] != rhs.mData[i]) {
          return false;
        }
      }

      return true;
    }
  }

  bool operator!=(const Span& rhs) const {
    if (mSize != rhs.mSize) {
      return true;
    }

    if constexpr (std::is_trivially_copyable_v<T> && std::has_unique_object_representations_v<T>) {
      return memcmp(mData, rhs.mData, mSize * sizeof(T)) != 0;
    }
    else {
      for (size_t i = 0; i < mSize; ++i) {
        if (mData[i] != rhs.mData[i]) {
          return true;
        }
      }

      return false;
    }
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

  Iterator rbegin() const {
    return Iterator(mData + mSize - 1);
  }

  Iterator rend() const {
    return Iterator(mData - 1);
  }

  private:
  T* mData = nullptr;
  size_t mSize = 0;
};

}
