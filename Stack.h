#pragma once

#include "ZBaseTypes.h"
#include "List.h"
#include "PlatformMemory.h"

namespace ZSharp {

template<typename T>
class Stack {
  public:

  Stack() {

  }

  ~Stack() {

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

  private:
  List<T> mData;
};

}
