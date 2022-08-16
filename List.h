#pragma once

#include "ZAssert.h"
#include "ZBaseTypes.h"

#include "PlatformMemory.h"

namespace ZSharp {

template<typename T>
class List final {
  private:

  struct Node {
    T* mValue = nullptr;
    Node* mNext = nullptr;
    Node* mPrev = nullptr;
  };

  public:

  class Iterator {
    public:
    Iterator(Node* data) : mPtr(data) {}

    T& operator++() {
      if (mPtr->mNext == nullptr) {
        Iterator temp(mPtr);
        mPtr = mPtr->mNext;
        return *temp;
      }
      else {
        mPtr = mPtr->mNext;
        return *(*this);
      }
    }

    T& operator++(int) {
      Iterator temp(mPtr);
      ++(*this);
      return *temp;
    }

    bool operator==(const Iterator& rhs) {
      return mPtr == rhs.mPtr;
    }

    bool operator!=(const Iterator& rhs) {
      return mPtr != rhs.mPtr;
    }

    T& operator*() const {
      return *(mPtr->mValue);
    }

    T* operator->() {
      return mPtr->mValue;
    }

    private:
    Node* mPtr;
  };

  List() {
  }

  ~List() {
    if (mHead == nullptr) {
      return;
    }

    while (mTail != mHead) {
      Node* prev = mTail->mPrev;
      DeleteNode(mTail);
      mTail = prev;
    }

    DeleteNode(mHead);
  }

  List(const List& rhs) {
    for (T& item : rhs) {
      Add(item);
    }
  }

  void operator=(const List& rhs) {
    if (this != &rhs) {
      for (T& item : rhs) {
        Add(item);
      }
    }
  }

  void Add(const T& item) {
    if (mTail == nullptr) {
      mHead = ConstructNode(mHead, item);
      mTail = mHead;
    }
    else {
      mTail = ConstructNode(mTail, item);
    }
  }

  void Remove(const T& item) {
    if (mHead == nullptr) {
      return;
    }

    for (Node* node = mTail; (node != mHead) && (node != nullptr); node = node->mPrev) {
      if (*(node->mValue) == item) {
        Node* prevNode = node->mPrev;
        Node* nextNode = node->mNext;

        if (node == mTail) {
          mTail = prevNode;
        }

        prevNode->mNext = nextNode;
        if (nextNode != nullptr) {
          nextNode->mPrev = prevNode;
        }

        DeleteNode(node);
      }
    }

    if (*(mHead->mValue) == item) {
      if (mHead->mNext == nullptr) {
        DeleteNode(mHead);
        mHead = nullptr;
      }
      else {
        Node* nextNode = mHead->mNext;
        DeleteNode(mHead);
        mHead = nextNode;
      }
    }
  }

  size_t Size() const {
    return mSize;
  }

  void Clear() {
    if (mHead == nullptr) {
      return;
    }

    while (mTail != mHead) {
      Node* prev = mTail->mPrev;
      DeleteNode(mTail);
      mTail = prev;
    }

    DeleteNode(mHead);

    mHead = nullptr;
  }

  Iterator begin() const {
    return Iterator(mHead);
  }

  Iterator end() const {
    if (mTail != nullptr) {
      return Iterator(mTail->mNext);
    }
    else {
      return Iterator(mTail);
    }
  }

  private:
  Node* mHead = nullptr;
  Node* mTail = nullptr;
  size_t mSize = 0;

  Node* ConstructNode(Node* prev, const T& value) {
    Node* node = new Node;
    node->mValue = new T(value);
    node->mPrev = prev;
    if (prev != nullptr) {
      prev->mNext = node;
    }

    mSize++;

    return node;
  }

  void DeleteNode(Node* node) {
    if (node->mValue != nullptr) {
      delete node->mValue;
    }

    delete node;

    --mSize;
  }
};

}
