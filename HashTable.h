#pragma once

#include "Array.h"
#include "HashFunctions.h"
#include "List.h"
#include "Pair.h"
#include "PlatformMemory.h"
#include "ZAssert.h"
#include "ZBaseTypes.h"

namespace ZSharp {

template<typename Key, typename Value, typename HashFunction = Hash<Key>>
class HashTable final {
  public:

  class Iterator {
    public:
    Iterator(Array<List<Pair<Key, Value>>>::Iterator begin, Array<List<Pair<Key, Value>>>::Iterator end) 
      : mStorageIter(begin), mStorageEnd(end), mBucketIter((*begin).begin()) {
      for (; mStorageIter != mStorageEnd; ++mStorageIter) {
        List<Pair<Key, Value>>& list = (*mStorageIter);

        if (list.Size() > 0) {
          mBucketIter = list.begin();
          break;
        }
      }
    }

    Iterator& operator++() {
      mBucketIter++;

      if (mBucketIter == (*mStorageIter).end()) {
        mStorageIter++;
        for (; mStorageIter != mStorageEnd; ++mStorageIter) {
          List<Pair<Key, Value>>& list = (*mStorageIter);

          if (list.Size() > 0) {
            mBucketIter = list.begin();
            break;
          }
        }
      }

      return *this;
    }

    Iterator operator++(int) {
      Iterator temp(*this);
      ++(*this);
      return temp;
    }

    bool operator==(const Iterator& rhs) {
      return mStorageIter == rhs.mStorageIter;
    }

    bool operator!=(const Iterator& rhs) {
      return mStorageIter != rhs.mStorageIter;
    }

    Pair<Key, Value>& operator*() const {
      return *mBucketIter;
    }

    Pair<Key, Value>* operator->() {
      return mBucketIter.operator->();
    }

    private:
    Array<List<Pair<Key, Value>>>::Iterator mStorageIter;
    Array<List<Pair<Key, Value>>>::Iterator mStorageEnd;
    List<Pair<Key, Value>>::Iterator mBucketIter;
  };

  HashTable()
    : mSize(0), mStorage(mMinCapacity) {

  }

  HashTable(size_t initialCapacity)
    : mSize(0), mStorage(initialCapacity) {

  }

  ~HashTable() {

  }

  HashTable(const HashTable& rhs)
    : mSize(rhs.mSize), mStorage(rhs.mStorage) {
  }
  
  void operator=(const HashTable& rhs) {
    if (this == &rhs) {
      return;
    }

    mSize = rhs.mSize;
    mStorage = rhs.mStorage;
  }

  Value& operator[](const Key& key) {
    {
      List<Pair<Key, Value>>& bucket = mStorage[HashedIndex(key)];

      bool found = false;
      for (Pair<Key, Value>& pair : bucket) {
        if (pair.mKey == key) {
          found = true;
          break;
        }
      }
      
      if (!found) {
        Pair<Key, Value> pair(key);
        bucket.Add(pair);
      }
    }

    // Must recompute hash in case of resizing.
    List<Pair<Key, Value>>& bucket = mStorage[HashedIndex(key)];
    Value& matchedValue = (*(bucket.begin())).mValue;
    for (Pair<Key, Value>& pair : bucket) {
      if (pair.mKey == key) {
        matchedValue = pair.mValue;
      }
    }

    return matchedValue;
  }

  bool Add(const Key& key, const Value& value) {
    InsertKeyValue(HashedIndex(key), key, value);

    if (mSize > Capacity()) {
      const size_t doubledCapacity = Capacity() * 2;
      size_t capacity = (doubledCapacity < mMinCapacity) ? mMinCapacity : doubledCapacity;
      Resize(capacity);
    }

    return true;
  }

  bool Remove(const Key& key) {
    bool wasRemoved = DeleteKey(HashedIndex(key), key);
    
    if (wasRemoved) {
      const size_t threshold = Capacity() / 4;
      if (mSize < threshold) {
        const size_t halvedCapacity = (Capacity() / 2);
        size_t capacity = (halvedCapacity < mMinCapacity) ? mMinCapacity : halvedCapacity;
        Resize(capacity);
      }
    }

    return wasRemoved;
  }

  bool HasKey(const Key& key) const {
    const List<Pair<Key, Value>>& bucket = mStorage[HashedIndex(key)];
    for (Pair<Key, Value>& pair : bucket) {
      if (pair.mKey == key) {
        return true;
      }
    }

    return false;
  }

  Value GetValue(const Key& key) const {
    const List<Pair<Key, Value>>& bucket = mStorage[HashedIndex(key)];
    for (Pair<Key, Value>& pair : bucket) {
      if (pair.mKey == key) {
        return pair.mValue;
      }
    }

    return Value();
  }

  void Resize(size_t size) {
    HashTable tempTable(size);
    for (List<Pair<Key, Value>>& bucket : mStorage) {
      for (Pair<Key, Value>& hashPair : bucket) {
        tempTable.Add(hashPair.mKey, hashPair.mValue);
      }
    }

    mSize = tempTable.mSize;
    mStorage = tempTable.mStorage;
  }

  size_t Size() const {
    return mSize;
  }

  size_t Capacity() const {
    return mStorage.Size();
  }

  Iterator begin() const {
    return Iterator(mStorage.begin(), mStorage.end());
  }

  Iterator end() const {
    return Iterator(mStorage.end(), mStorage.end());
  }

  private:
  const size_t mMinCapacity = 4096;
  size_t mSize;
  Array<List<Pair<Key, Value>>> mStorage;

  uint32 HashedIndex(const Key& key) const {
    HashFunction hashFunctor;
    uint32 hash = hashFunctor(key);
    return hash % Capacity();
  }

  void InsertKeyValue(size_t index, const Key& key, const Value& value) {
    List<Pair<Key, Value>>& bucket = mStorage[index];
    if (bucket.Size() == 0) {
      Pair<Key, Value> pair(key, value);
      bucket.Add(pair);
      ++mSize;
    }
    else {
      bool foundMatch = false;
      for (Pair<Key, Value>& pair : bucket) {
        if (pair.mKey == key) {
          pair.mValue = value;
          foundMatch = true;
          break;
        }
      }

      if (!foundMatch) {
        Pair<Key, Value> pair(key, value);
        bucket.Add(pair);
        ++mSize;
      }
    }
  }

  bool DeleteKey(size_t index, const Key& key) {
    List<Pair<Key, Value>>& bucket = mStorage[index];

    Pair<Key, Value> pairToRemove;

    bool found = false;
    for (Pair<Key, Value>& pair : bucket) {
      if (pair.mKey == key) {
        found = true;
        pairToRemove = pair;
        break;
      }
    }

    if (found && bucket.Remove(pairToRemove)) {
      --mSize;
      return true;
    }
    else {
      return false;
    }
  }
};

}
