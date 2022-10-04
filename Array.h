#pragma once

#include "ZBaseTypes.h"
#include "ZAssert.h"
#include "PlatformMemory.h"

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
    Free();
  }

  Array(const Array& rhs) {
    FreshAllocNoInit(rhs.mSize);
    for (size_t i = 0; i < mSize; ++i) {
      new(mData + i) T(rhs[i]);
    }
  }

  Array(const Array&&) = delete;

  void operator=(const Array& rhs) {
    if (this != &rhs && rhs.mSize > 0) {
      Free();
      FreshAllocNoInit(rhs.mSize);
      for (size_t i = 0; i < mSize; ++i) {
        new(mData + i) T(rhs[i]);
      }
    }
  }

  void operator=(const Array&&) = delete;

  T& operator[](size_t index) {
    ZAssert(index < mSize);
    return mData[index];
  }

  const T& operator[](size_t index) const {
    ZAssert(index < mSize);
    return mData[index];
  }

  T* GetData() {
    ZAssert(mData != nullptr);
    return mData;
  }

  const T* GetData() const {
    ZAssert(mData != nullptr);
    return mData;
  }

  void Clear() {
    Free();
  }

  size_t Size() const {
    return mSize;
  }

  size_t Capacity() const {
    return mCapacity;
  }

  bool IsEmpty() const {
    return mSize == 0;
  }

  void Resize(size_t size) {
    if (mData == nullptr) {
      FreshAlloc(size);
    }
    else if (mSize != size) {
      mCapacity = size * 2;

      if (mSize < size) {
        mData = static_cast<T*>(PlatformReAlloc(mData, mCapacity * sizeof(T)));
        for (size_t i = mSize; i < size; ++i) {
          new(mData + i) T();
        }
      }
      else {
        for (size_t i = size; i < mSize; ++i) {
          (mData + i)->~T();
        }

        mData = static_cast<T*>(PlatformReAlloc(mData, mCapacity * sizeof(T)));
      }

      mSize = size;
    }
  }

  void PushBack(const T& data) {
    if (mCapacity <= mSize) {
      Resize(mSize + 1);
      mData[mSize - 1] = data;
    }
    else {
      new(mData + mSize) T(data);
      ++mSize;
    }
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
  size_t mCapacity = 0;
  size_t mPadding = 0; // TODO: Makes sure this can align on a 16 byte boundary.

  void FreshAlloc(size_t size) {
    const size_t slack = size * 2;
    const size_t totalSize = sizeof(T) * slack;

    mData = static_cast<T*>(PlatformMalloc(totalSize));
    mSize = size;
    mCapacity = slack;

    for (size_t i = 0; i < mSize; ++i) {
      new(mData + i) T();
    }
  }

  void FreshAllocNoInit(size_t size) {
    const size_t slack = size * 2;
    const size_t totalSize = sizeof(T) * slack;
    mData = static_cast<T*>(PlatformMalloc(totalSize));
    mSize = size;
    mCapacity = slack;
  }

  void Free() {
    if (mData != nullptr) {
      for (size_t i = 0; i < mSize; ++i) {
        (mData + i)->~T();
      }

      PlatformFree(mData);
      mData = nullptr;
      mSize = 0;
    }
  }
};

}