#pragma once

#include "ZAssert.h"
#include "ZBaseTypes.h"

#include "PlatformMemory.h"

#include <initializer_list>

namespace ZSharp {

template<typename T>
class List final {
  private:

  struct Node {
    T* mValue = nullptr;
    Node* mNext = nullptr;
    Node* mPrev = nullptr;

    Node(const T& value, Node* prev) 
      : mValue(new T(value)), mPrev(prev) {
      if (prev != nullptr) {
        prev->mNext = this;
      }
    }

    ~Node() {
      if (mValue != nullptr) {
        delete mValue;
      }
    }
  };

  public:

  class Iterator {
    public:
    Iterator(Node* data) : mPtr(data) {}

    Iterator& operator++() {
      mPtr = mPtr->mNext;
      return *this;
    }

    Iterator operator++(int) {
      Iterator temp(mPtr);
      ++(*this);
      return *temp;
    }

    Iterator& operator--() {
      mPtr = mPtr->mPrev;
      return *this;
    }

    Iterator operator--(int) {
      Iterator temp(mPtr);
      --(*this);
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

  List(std::initializer_list<T> initList) {
    for (const T& item : initList) {
      Add(item);
    }
  }

  ~List() {
    Clear();
  }

  List(const List& rhs) {
    for (const T& item : rhs) {
      Add(item);
    }
  }

  void operator=(const List& rhs) {
    if (this != &rhs) {
      Clear();
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

  bool Remove(const T& item) {
    if (mHead == nullptr) {
      return false;
    }

    bool wasRemoved = false;

    for (Node* node = mTail; (node != mHead) && (node != nullptr); node = node->mPrev) {
      if (item == (*(node->mValue))) {
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
        wasRemoved = true;
      }
    }

    if (item == (*(mHead->mValue))) {
      if (mHead->mNext == nullptr) {
        DeleteNode(mHead);
        wasRemoved = true;
        mHead = nullptr;
      }
      else {
        Node* nextNode = mHead->mNext;
        DeleteNode(mHead);
        wasRemoved = true;
        mHead = nextNode;
      }
    }

    return wasRemoved;
  }

  void RemoveFront() {
    if (mHead == nullptr) {
      return;
    }

    Node* nextNode = mHead->mNext;
    DeleteNode(mHead);
    mHead = nextNode;
    mHead->mPrev = nullptr;
  }

  void RemoveBack() {
    if (mTail == nullptr) {
      return;
    }

    Node* prevNode = mTail->mPrev;
    DeleteNode(mTail);
    mTail = prevNode;
    mTail->mNext = nullptr;
  }

  bool Contains(const T& item) const {
    for (const T& storedItem : *this) {
      if (item == storedItem) {
        return true;
      }
    }

    return false;
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
    return Iterator(nullptr);
  }

  Iterator rbegin() const {
    return Iterator(mTail);
  }

  Iterator rend() const {
    return Iterator(nullptr);
  }

  private:
  Node* mHead = nullptr;
  Node* mTail = nullptr;
  size_t mSize = 0;

  Node* ConstructNode(Node* prev, const T& value) {
    Node* node = new Node(value, prev);
    mSize++;
    return node;
  }

  void DeleteNode(Node* node) {
    delete node;
    --mSize;
  }
};

}
