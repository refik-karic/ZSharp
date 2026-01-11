#pragma once

#include "ZAssert.h"
#include "ZBaseTypes.h"
#include "PlatformMemory.h"
#include "Common.h"
#include "MoveHelpers.h"

#include <initializer_list>
#include <type_traits>

namespace ZSharp {

template<typename T, size_t N>
class RingBuffer final {
  public:

  class Iterator {
    public:
    Iterator(T* begin, T* end, T* current, size_t remaining) : mBegin(begin), mEnd(end), mCurrent(current), mRemaining(remaining) {}

    Iterator& operator++() {
      mCurrent++;
      mRemaining--;
      if (mCurrent >= mEnd) {
        mCurrent = mBegin;
      }

      return *this;
    }

    Iterator operator++(int) {
      Iterator temp(*this);
      ++(*this);
      return temp;
    }

    Iterator& operator--() {
      mCurrent--;
      mRemaining--;
      if (mCurrent <= mBegin) {
        mCurrent = mEnd;
      }

      return *this;
    }

    Iterator operator--(int) {
      Iterator temp(*this);
      --(*this);
      return temp;
    }

    bool operator==(const Iterator& rhs) {
      return mCurrent == rhs.mCurrent && mRemaining == 0;
    }

    bool operator!=(const Iterator& rhs) {
      return mCurrent != rhs.mCurrent || mRemaining != 0;
    }

    T& operator*() const {
      return *mCurrent;
    }

    T* operator->() {
      return mCurrent;
    }

    private:
    T* mBegin;
    T* mEnd;
    T* mCurrent;
    size_t mRemaining;
  };

  RingBuffer() {
    mData = (T*)PlatformCalloc(N * sizeof(T));
  }

  RingBuffer(const std::initializer_list<T>& initList) {
    mData = (T*)PlatformCalloc(N * sizeof(T));

    if (N < initList.size()) {
      // Avoid inserting anything into the buffer if we don't have enough space.
      // There's no way for us to return something to the caller in a constructor.
      ZAssert(false);
      return;
    }

    const size_t size = initList.size();

    size_t i = 0;
    for (const T& item : initList) {
      new(mData + i) T(item);
      ++i;
    }

    if (size == N) {
      mFull = true;
    }
    else {
      mHead += size;
    }
  }

  ~RingBuffer() {
    if (!mData) {
      return;
    }

    if constexpr (!(std::is_trivially_default_constructible_v<T>) && !(std::is_trivially_destructible_v<T>)) {
      const size_t size = Size();
      for (size_t i = 0; i < size; ++i, ++mTail) {
        if (mTail == N) {
          mTail = 0;
        }

        (mData + mTail)->~T();
      }
    }

    if (mData) {
      PlatformFree(mData);
    }
  }

  RingBuffer(const RingBuffer& rhs) {
    mData = (T*)PlatformCalloc(N * sizeof(T));
    mHead = rhs.mHead;
    mTail = rhs.mTail;
    mFull = rhs.mFull;

    if constexpr (std::is_trivially_default_constructible_v<T> && std::is_trivially_destructible_v<T>) {
      memcpy(mData, rhs.mData, N * sizeof(T));
    }
    else {
      const size_t size = Size();

      for (size_t i = 0; i < size; ++i, ++mTail) {
        if (mTail == N) {
          mTail = 0;
        }

        new(mData + mTail) T(rhs.mData[mTail]);
      }
    }

    mHead = rhs.mHead;
    mTail = rhs.mTail;
    mFull = rhs.mFull;
  }

  RingBuffer(RingBuffer&& rhs) {
    mData = rhs.mData;
    mHead = rhs.mHead;
    mTail = rhs.mTail;
    mFull = rhs.mFull;
    rhs.mData = nullptr;
  }

  void operator=(const RingBuffer& rhs) {
    if (this == &rhs) {
      return;
    }

    Clear();

    mHead = rhs.mHead;
    mTail = rhs.mTail;
    mFull = rhs.mFull;

    if constexpr (std::is_trivially_default_constructible_v<T> && std::is_trivially_destructible_v<T>) {
      memcpy(mData, rhs.mData, N * sizeof(T));
    }
    else {
      const size_t size = Size();

      for (size_t i = 0; i < size; ++i, ++mTail) {
        if (mTail == N) {
          mTail = 0;
        }

        new(mData + mTail) T(rhs.mData[mTail]);
      }
    }

    mHead = rhs.mHead;
    mTail = rhs.mTail;
    mFull = rhs.mFull;
  }

  void operator=(RingBuffer&& rhs) {
    mData = rhs.mData;
    mHead = rhs.mHead;
    mTail = rhs.mTail;
    mFull = rhs.mFull;
    rhs.mData = nullptr;
  }

  bool operator==(const RingBuffer& rhs) const {
    if (this == &rhs) {
      return true;
    }

    Iterator lhsBegin = begin();
    Iterator lhsEnd = end();
    Iterator rhsBegin = rhs.begin();
    while (lhsBegin != lhsEnd) {
      if (lhsBegin != rhsBegin) {
        return false;
      }

      ++lhsBegin;
      ++rhsBegin;
    }
  }

  T& operator[](size_t index) {
    ZAssert(index < Size());
    return mData[(mTail + index) % N];
  }

  const T& operator[](size_t index) const {
    ZAssert(index < Size());
    return mData[(mTail + index) % N];
  }

  template<typename... Args>
  void PushBack(Args&&... args) {
    if (IsFull()) {
      return;
    }

    new(mData + mHead) T(Forward<Args>(args)...);
    
    ++mHead;

    mFull = Size() == N;

    if (mHead == N) {
      mHead = 0;
    }
  }

  template<typename... Args>
  void PushBackOverwrite(Args&&... args) {
    if (IsFull()) {
      if constexpr (!(std::is_trivially_default_constructible_v<T>) && !(std::is_trivially_destructible_v<T>)) {
        (mData + mHead)->~T();
      }
    }

    new(mData + mHead) T(Forward<Args>(args)...);

    ++mHead;

    mFull = Size() == N;

    if (mHead == N) {
      mHead = 0;
    }
  }

  void PopBack() {
    if (!IsEmpty()) {
      if constexpr (!(std::is_trivially_default_constructible_v<T>) && !(std::is_trivially_destructible_v<T>)) {
        (mData + mTail)->~T();
      }

      ++mTail;
      mFull = false;
    }
  }

  size_t Capacity() const {
    return N;
  }

  size_t Size() const {
    if (mFull) {
      return N;
    }
    else if (mHead < mTail) {
      return N + mHead - mTail;
    }
    else {
      return mHead - mTail;
    }
  }

  bool IsEmpty() const {
    return mHead == mTail && !mFull;
  }

  bool IsFull() const {
    return mFull;
  }

  void Clear() {
    if constexpr (std::is_trivially_default_constructible_v<T> && std::is_trivially_destructible_v<T>) {
      memset(mData, 0, N * sizeof(T));
    }
    else {
      const size_t size = Size();
      for (size_t i = 0; i < size; ++i, ++mTail) {
        if (mTail == N) {
          mTail = 0;
        }

        (mData + mTail)->~T();
      }
    }

    mHead = 0;
    mTail = 0;
    mFull = false;
  }

  T* GetData() const {
    return mData + mTail;
  }

  Iterator begin() const {
    return Iterator(mData, mData + N, mData + mTail, Size());
  }

  Iterator end() const {
    return Iterator(mData, mData + N, mData + mHead, 0);
  }

  Iterator rbegin() const {
    return Iterator(mData, mData + N, mData + mHead, Size());
  }

  Iterator rend() const {
    return Iterator(mData, mData + N, mData + mTail, 0);
  }

  private:
  T* mData = nullptr;
  size_t mHead = 0;
  size_t mTail = 0;
  bool mFull = false;
};

}
