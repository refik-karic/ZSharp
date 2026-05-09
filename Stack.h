#pragma once

#include "ZBaseTypes.h"
#include "List.h"
#include "MoveHelpers.h"

#include <initializer_list>

namespace ZSharp {

template<typename T>
class Stack final {
  public:

  class Iterator {
    public:

    Iterator(List<T>::Iterator iter) : mIter(iter) {

    }

    Iterator& operator++() {
      mIter--;
      return *this;
    }

    Iterator& operator--() {
      mIter++;
      return *this;
    }

    Iterator operator++(int) {
      Iterator temp(*this);
      --(*this);
      return temp;
    }

    Iterator operator--(int) {
      Iterator temp(*this);
      ++(*this);
      return temp;
    }

    bool operator==(const Iterator& rhs) {
      return mIter == rhs.mIter;
    }

    bool operator!=(const Iterator& rhs) {
      return mIter != rhs.mIter;
    }

    T& operator*() const {
      return *mIter;
    }

    T* operator->() {
      return mIter.operator->();
    }

    private:
    List<T>::Iterator mIter;
  };

  Stack() {};

  Stack(const Stack& rhs) 
    : mData(rhs.mData) {

  }

  Stack(const std::initializer_list<T>& initList) {
    for (const T& item : initList) {
      Push(item);
    }
  }

  Stack(Stack&& rhs) : mData(Move(rhs.mData)) {
  }

  ~Stack() {}

  void operator=(const Stack& rhs) {
    if (this != &rhs) {
      mData = rhs.mData;
    }
  }

  void operator=(Stack&& rhs) {
    mData = Move(rhs.mData);
  }

  bool operator==(const Stack& rhs) const {
    return mData == rhs.mData;
  }

  void Push(const T& item) {
    mData.Add(item);
  }

  void Pop() {
    mData.RemoveBack();
  }

  T& Peek() {
    if (Size() == 0) {
      T empty{};
      Push(empty);
    }

    return *mData.rbegin();
  }

  size_t Size() const {
    return mData.Size();
  }

  Iterator begin() const {
    return Iterator(mData.rbegin());
  }

  Iterator end() const {
    return Iterator(mData.rend());
  }

  Iterator rbegin() const {
    return Iterator(mData.begin());
  }

  Iterator rend() const {
    return Iterator(mData.end());
  }

  private:
  List<T> mData;
};

}
