#pragma once

#include "ZAssert.h"
#include "ZBaseTypes.h"
#include "PlatformMemory.h"
#include "ISerializable.h"
#include "Common.h"
#include "MoveHelpers.h"

#include <cstring>
#include <initializer_list>
#include <type_traits>

namespace ZSharp {

template<typename T>
class Array final : public ISerializable {
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
    
    if constexpr (std::is_trivially_default_constructible_v<T> && std::is_trivially_destructible_v<T>) {
      memcpy(mData, rhs.mData, mSize * sizeof(T));
    }
    else {
      for (size_t i = 0; i < mSize; ++i) {
        new(mData + i) T(rhs[i]);
      }
    }
  }

  Array(Array&& rhs) {
    mData = rhs.mData;
    mSize = rhs.mSize;
    mCapacity = rhs.mCapacity;
    rhs.mData = nullptr;
  }

  void operator=(const Array& rhs) {
    if (this != &rhs && rhs.mSize > 0) {
      Free();
      FreshAllocNoInit(rhs.mSize);

      if constexpr (std::is_trivially_default_constructible_v<T> && std::is_trivially_destructible_v<T>) {
        memcpy(mData, rhs.mData, mSize * sizeof(T));
      }
      else {
        for (size_t i = 0; i < mSize; ++i) {
          new(mData + i) T(rhs[i]);
        }
      }
    }
  }

  void operator=(Array&& rhs) {
    mData = rhs.mData;
    mSize = rhs.mSize;
    mCapacity = rhs.mCapacity;
    rhs.mData = nullptr;
  }

  bool operator==(const Array& rhs) const {
    if (mSize != rhs.mSize) {
      return false;
    }

    if constexpr (std::is_trivially_default_constructible_v<T> && std::is_trivially_destructible_v<T>) {
      if (memcmp(mData, rhs.mData, mSize * sizeof(T))) {
        return false;
      }
    }
    else {
      for (size_t i = 0; i < mSize; ++i) {
        if (mData[i] != rhs.mData[i]) {
          return false;
        }
      }
    }

    return true;
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

        if constexpr (std::is_trivially_default_constructible_v<T> && std::is_trivially_destructible_v<T>) {
          memset(mData + mSize, 0, (size - mSize) * sizeof(T));
        }
        else {
          for (size_t i = mSize; i < size; ++i) {
            new(mData + i) T();
          }
        }
      }
      else {
        if constexpr (!(std::is_trivially_default_constructible_v<T>) && !(std::is_trivially_destructible_v<T>)) {
          for (size_t i = size; i < mSize; ++i) {
            (mData + i)->~T();
          }
        }

        mData = static_cast<T*>(PlatformReAlloc(mData, mCapacity * sizeof(T)));
      }

      mSize = size;
    }
  }

  T& PushBack(const T& data) {
    T* result;

    if (mCapacity <= mSize) {
      size_t currentSize = mSize;
      ResizeNoInit(mSize + 1);
      result = new(mData + currentSize) T(data);
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
      const size_t currentSize = mSize;
      ResizeNoInit(mSize + 1);
      result = new(mData + currentSize) T(Forward<Args>(args)...);
    }
    else {
      result = new(mData + mSize) T(Forward<Args>(args)...);
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

  Iterator rbegin() const {
    return Iterator(mData + mSize - 1);
  }

  Iterator rend() const {
    return Iterator(mData - 1);
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

    if constexpr (std::is_base_of_v<ISerializable, T>) {
      for (size_t i = 0; i < mSize; ++i) {
        mData[i].Serialize(serializer);
      }
    }
    else {
      const size_t sizeBytes = mSize * sizeT;
      serializer.Serialize(mData, sizeBytes);
    }
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

    if constexpr (std::is_base_of_v<ISerializable, T>) {
      FreshAlloc(savedSize);
      for (size_t i = 0; i < savedSize; ++i) {
        mData[i].Deserialize(deserializer);
      }
    }
    else {
      FreshAllocNoInit(savedSize);
      const size_t sizeBytes = savedSize * sizeof(T);
      deserializer.Deserialize(mData, sizeBytes);
    }
  }

  private:
  T* mData = nullptr;
  size_t mSize = 0;
  size_t mCapacity = 0;

  void FreshAlloc(size_t size) {
    const size_t slack = size * 2;
    const size_t totalSize = sizeof(T) * slack;

    mSize = size;
    mCapacity = slack;

    if constexpr (std::is_trivially_default_constructible_v<T> && std::is_trivially_destructible_v<T>) {
      mData = static_cast<T*>(PlatformCalloc(totalSize));
    }
    else {
      mData = static_cast<T*>(PlatformMalloc(totalSize));
      for (size_t i = 0; i < mSize; ++i) {
        new(mData + i) T();
      }
    }
  }

  void FreshAllocNoInit(size_t size) {
    const size_t slack = size * 2;
    const size_t totalSize = sizeof(T) * slack;
    mData = static_cast<T*>(PlatformMalloc(totalSize));
    mSize = size;
    mCapacity = slack;
  }

  void ResizeNoInit(size_t size) {
    if (mData == nullptr) {
      FreshAllocNoInit(size);
    }
    else if (mSize != size) {
      mCapacity = size * 2;

      if (mSize < size) {
        mData = static_cast<T*>(PlatformReAlloc(mData, mCapacity * sizeof(T)));
      }
      else {
        ZAssert(false);
      }

      mSize = size;
    }
  }

  void Free() {
    if (mData != nullptr) {
      if constexpr (!(std::is_trivially_default_constructible_v<T>) && !(std::is_trivially_destructible_v<T>)) {
        for (size_t i = 0; i < mSize; ++i) {
          (mData + i)->~T();
        }
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
