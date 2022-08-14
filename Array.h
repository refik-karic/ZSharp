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
      Resize(rhs.mSize);
      for (size_t i = 0; i < rhs.mSize; ++i) {
        mData[i] = rhs[i];
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

  bool IsEmpty() const {
    return mSize == 0;
  }

  void Resize(size_t size) {
    if (mData == nullptr) {
      FreshAlloc(size);
    }
    else if (mSize != size) {
      if (mSize < size) {
        mData = static_cast<T*>(PlatformReAlloc(mData, size * sizeof(T)));
        for (size_t i = mSize; i < size; ++i) {
          new(mData + i) T();
        }
      }
      else {
        const size_t totalSize = sizeof(T) * size;
        T* newAlloc = static_cast<T*>(PlatformMalloc(totalSize));

        for (size_t i = 0; i < size; ++i) {
          new(newAlloc + i) T(mData[i]);
        }

        for (size_t i = size; i < mSize; ++i) {
          (mData + i)->~T();
        }

        PlatformFree(mData);
        mData = newAlloc;
      }

      mSize = size;
    }
  }

  void PushBack(const T& data) {
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
    const size_t totalSize = sizeof(T) * size;
    mData = static_cast<T*>(PlatformMalloc(totalSize));
    mSize = size;

    for (size_t i = 0; i < mSize; ++i) {
      new(mData + i) T();
    }
  }

  void FreshAllocNoInit(size_t size) {
    const size_t totalSize = sizeof(T) * size;
    mData = static_cast<T*>(PlatformMalloc(totalSize));
    mSize = size;
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
