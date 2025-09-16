#pragma once

#include "ZAssert.h"
#include "ZBaseTypes.h"
#include "PlatformMemory.h"
#include "ISerializable.h"

#include <initializer_list>
#include <type_traits>

namespace ZSharp {

template<typename T>
class List final : public ISerializable {
  private:

  struct Node {
    T mValue;
    Node* mNext = nullptr;
    Node* mPrev = nullptr;

    Node(const T& value, Node* prev) 
      : mValue(value), mPrev(prev) {
      if (prev != nullptr) {
        prev->mNext = this;
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
      return temp;
    }

    Iterator& operator--() {
      mPtr = mPtr->mPrev;
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
      return mPtr->mValue;
    }

    T* operator->() {
      return &mPtr->mValue;
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

  bool operator==(const List& rhs) const {
    if (mSize != rhs.mSize) {
      return false;
    }

    Iterator lhsIter = begin();
    Iterator lhsEnd = end();
    Iterator rhsIter = rhs.begin();
    while (lhsIter != lhsEnd) {
      if ((*lhsIter) != *(rhsIter)) {
        return false;
      }

      lhsIter++;
      rhsIter++;
    }

    return true;
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
      if (item == node->mValue) {
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

    if (item == mHead->mValue) {
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

    if (mHead == mTail) {
      DeleteNode(mHead);
      mHead = nullptr;
      mTail = nullptr;
    }
    else {
      Node* nextNode = mHead->mNext;
      DeleteNode(mHead);
      mHead = nextNode;
      mHead->mPrev = nullptr;
    }
  }

  void RemoveBack() {
    if (mTail == nullptr) {
      return;
    }

    if (mHead == mTail) {
      DeleteNode(mTail);
      mHead = nullptr;
      mTail = nullptr;
    }
    else {
      Node* prevNode = mTail->mPrev;
      DeleteNode(mTail);
      mTail = prevNode;
      mTail->mNext = nullptr;
    }
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

  bool IsEmpty() const {
    return mSize == 0;
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
    mTail = nullptr;
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
      Node* node = mHead;
      for (size_t i = 0; i < mSize; ++i, node = node->mNext) {
        node->mValue.Serialize(serializer);
      }
    }
    else {
      Node* node = mHead;
      for (size_t i = 0; i < mSize; ++i, node = node->mNext) {
        serializer.Serialize(&(node->mValue), sizeT);
      }
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
      Clear();
    }

    if constexpr (std::is_base_of_v<ISerializable, T>) {
      T item;
      for (size_t i = 0; i < savedSize; ++i) {
        item.Deserialize(deserializer);
        Add(item);
      }
    }
    else {
      uint8 buffer[sizeof(T)];
      for (size_t i = 0; i < savedSize; ++i) {
        deserializer.Deserialize(buffer, sizeof(T));
        Add(*((T*)buffer));
      }
    }
  }

  /*
  Sort using in-place Merge Sort.
  Order is not guaranteed.
  */
  void Sort() {
    // Must have > 2 elements to do anything useful.
    if (mSize < 2) {
      return;
    }

    mHead = MergeSort(mHead);
    mHead->mPrev = nullptr;
  }

  template<typename Compare>
  void Sort(const Compare& comp) {
    // Must have > 2 elements to do anything useful.
    if (mSize < 2) {
      return;
    }

    mHead = MergeSort(mHead, comp);
    mHead->mPrev = nullptr;
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

  Node* GetMiddle(Node* node) {
    if (node == nullptr) {
      return node;
    }

    Node* slow = node;
    Node* fast = node;
    while (fast->mNext != nullptr && fast->mNext->mNext != nullptr) {
      slow = slow->mNext;
      fast = fast->mNext->mNext;
    }

    return slow;
  }

  Node* MergeSort(Node* node) {
    if (node == nullptr || node->mNext == nullptr) {
      return node;
    }

    Node* middle = GetMiddle(node);
    Node* rightSide = middle->mNext;
    middle->mNext = nullptr;

    return SortNodes(MergeSort(node), MergeSort(rightSide));
  }

  template<typename Compare>
  Node* MergeSort(Node* node, const Compare& comp) {
    if (node == nullptr || node->mNext == nullptr) {
      return node;
    }

    Node* middle = GetMiddle(node);
    Node* rightSide = middle->mNext;
    middle->mNext = nullptr;

    return SortNodes(MergeSort(node, comp), MergeSort(rightSide, comp), comp);
  }

  Node* SortNodes(Node* a, Node* b) {
    Node* result = nullptr;

    if (a == nullptr) {
      return b;
    }
    else if (b == nullptr) {
      return a;
    }

    if (a->mValue < b->mValue) {
      result = a;
      result->mNext = SortNodes(a->mNext, b);
    }
    else {
      result = b;
      result->mNext = SortNodes(a, b->mNext);
    }

    // Keep back references up to date and tail at the end.
    if(result->mNext != nullptr) {
      result->mNext->mPrev = result;
    }

    if (a->mValue > mTail->mValue) {
      mTail = a;
    }

    if (b->mValue > mTail->mValue) {
      mTail = b;
    }

    return result;
  }

  template<typename Compare>
  Node* SortNodes(Node* a, Node* b, const Compare& comp) {
    Node* result = nullptr;

    if (a == nullptr) {
      return b;
    }
    else if (b == nullptr) {
      return a;
    }

    if (comp(a->mValue, b->mValue)) {
      result = a;
      result->mNext = SortNodes(a->mNext, b, comp);
    }
    else {
      result = b;
      result->mNext = SortNodes(a, b->mNext, comp);
    }

    // Keep back references up to date and tail at the end.
    if (result->mNext != nullptr) {
      result->mNext->mPrev = result;
    }

    if (!comp(a->mValue, mTail->mValue)) {
      mTail = a;
    }

    if (!comp(b->mValue, mTail->mValue)) {
      mTail = b;
    }

    return result;
  }
};

}
