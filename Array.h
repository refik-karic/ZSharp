#pragma once

#include "Common.h"
#include "ZBaseTypes.h"
#include "ZAssert.h"
#include "PlatformMemory.h"
#include "Serializer.h"
#include "ISerializable.h"

#include <initializer_list>

#pragma warning(disable : 4324)

namespace ZSharp {

template<typename T>
class alignas(32) Array final : public ISerializable {
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

  Array(std::initializer_list<T> initList) {
    const size_t size = initList.size();
    FreshAllocNoInit(size);
    size_t i = 0;
    for (const T& item : initList) {
      new(mData + i) T(item);
      ++i;
    }
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

  void operator=(const Array& rhs) {
    if (this != &rhs && rhs.mSize > 0) {
      Free();
      FreshAllocNoInit(rhs.mSize);
      for (size_t i = 0; i < mSize; ++i) {
        new(mData + i) T(rhs[i]);
      }
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

  bool Contains(const T& item) const {
    for (size_t i = 0; i < mSize; ++i) {
      if (mData[i] == item) {
        return true;
      }
    }

    return false;
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

  T& PushBack(const T& data) {
    T* result;

    if (mCapacity <= mSize) {
      Resize(mSize + 1);
      mData[mSize - 1] = data;
      result = mData + mSize - 1;
    }
    else {
      result = new(mData + mSize) T(data);
      ++mSize;
    }

    return *result;
  }

  template<typename... Args>
  T& EmplaceBack(Args&&... args) {
    T* result;

    if (mCapacity <= mSize) {
      Resize(mSize + 1);
      result = new(mData + mSize - 1) T(args...);
    }
    else {
      result = new(mData + mSize) T(args...);
      ++mSize;
    }

    return *result;
  }

  /*
  Sort using in-place Quick Sort.
  Order is not guaranteed.
  */
  void Sort() {
    if (IsEmpty()) {
      return;
    }

    Quicksort(mData, mData + (mSize - 1));
  }

  template<typename Compare>
  void Sort(const Compare& comp) {
    Quicksort(mData, mData + (mSize - 1), comp);
  }

  Iterator begin() const {
    return Iterator(mData);
  }

  Iterator end() const {
    return Iterator(mData + mSize);
  }

  virtual void Serialize(ISerializer& serializer) override {
    // Must write size even if 0.
    // This makes it so Deserialize can check for size 0 and return.
    serializer.Serialize(&mSize, sizeof(mSize));

    if (mSize == 0) {
      return;
    }

    // Write out size of element in case something changes.
    const size_t sizeT = sizeof(T);
    serializer.Serialize(&sizeT, sizeof(sizeT));

    const size_t sizeBytes = mSize * sizeT;
    serializer.Serialize(mData, sizeBytes);
  }

  virtual void Deserialize(IDeserializer& deserializer) override {
    size_t savedSize = 0;
    deserializer.Deserialize(&savedSize, sizeof(savedSize));

    if (savedSize == 0) {
      return;
    }

    size_t savedTSize = 0;
    deserializer.Deserialize(&savedTSize, sizeof(savedTSize));

    if (savedTSize != sizeof(T)) {
      ZAssert(false);
      return;
    }

    if (mSize != 0) {
      Free();
    }

    FreshAllocNoInit(savedSize);
    const size_t sizeBytes = savedSize * sizeof(T);
    deserializer.Deserialize(mData, sizeBytes);
  }

  private:
  T* mData = nullptr;
  size_t mSize = 0;
  size_t mCapacity = 0;

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
      mCapacity = 0;
    }
  }

  void Quicksort(T* start, T* end) {
    T* pivot = end;
    T* secondPivot = nullptr;

    if (start == end) {
      return;
    }

    for (T* i = start; i != pivot; ++i) {
      const T& pivotValue = (*pivot);
      if (((*i) > pivotValue) && (secondPivot == nullptr)) {
        secondPivot = i;
      }
      else if (((*i) < pivotValue) && (secondPivot != nullptr)) {
        Swap(*secondPivot, *i);
        secondPivot++;
      }
    }

    if (secondPivot != nullptr) {
      Swap(*secondPivot, *pivot);
      Quicksort(start, secondPivot);
      Quicksort(secondPivot + 1, end);
    }
    else {
      Quicksort(start, pivot - 1);
      Quicksort(pivot, end);
    }
  }

  template<typename Compare>
  void Quicksort(T* start, T* end, const Compare& comp) {
    T* pivot = end;
    T* secondPivot = nullptr;

    if (start == end) {
      return;
    }

    for (T* i = start; i != pivot; ++i) {
      const T& pivotValue = (*pivot);
      if (comp(pivotValue, (*i)) && (secondPivot == nullptr)) {
        secondPivot = i;
      }
      else if (comp((*i), pivotValue) && (secondPivot != nullptr)) {
        Swap(*secondPivot, *i);
        secondPivot++;
      }
    }

    if (secondPivot != nullptr) {
      Swap(*secondPivot, *pivot);
      Quicksort(start, secondPivot, comp);
      Quicksort(secondPivot + 1, end, comp);
    }
    else {
      Quicksort(start, pivot - 1, comp);
      Quicksort(pivot, end, comp);
    }
  }
};

}
