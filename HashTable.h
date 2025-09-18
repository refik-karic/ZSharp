#pragma once

#include "Array.h"
#include "CommonMath.h"
#include "HashFunctions.h"
#include "Pair.h"
#include "PlatformMemory.h"
#include "ZAssert.h"
#include "ZBaseTypes.h"

namespace ZSharp {

template<typename Key, typename Value, typename HashFunction = Hash<Key>>
class HashTable final {
  private:
  class TableEntry final {
    public:
    bool occupied = false;
    uint8 kvpBuff[sizeof(Pair<Key, Value>)];
  };
  
  public:

  class Iterator {
    public:
    Iterator(typename Array<TableEntry>::Iterator iter,
      typename Array<TableEntry>::Iterator end,
      bool reverse)
      : mIter(iter), mEnd(end) {
      if (reverse) {
        for (; mIter != mEnd && (!mIter->occupied); --mIter) {}
      }
      else {
        for (; mIter != mEnd && (!mIter->occupied); ++mIter) {}
      }
    }

    Iterator& operator++() {
      mIter++;

      for (; mIter != mEnd && (!mIter->occupied); ++mIter) {}

      return *this;
    }

    Iterator& operator--() {
      mIter--;

      for (; mIter != mEnd && (!mIter->occupied); --mIter) {}

      return *this;
    }

    Iterator operator++(int) {
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

    Pair<Key, Value>& operator*() const {
      return *((Pair<Key, Value>*)((*mIter).kvpBuff));
    }

    Pair<Key, Value>* operator->() {
      return (Pair<Key, Value>*)((*mIter).kvpBuff);
    }

    private:
    typename Array<TableEntry>::Iterator mIter;
    typename Array<TableEntry>::Iterator mEnd;
  };

  HashTable()
    : mSize(0), mMinCapacity(4096), mStorage(4096)  {
  }

  HashTable(size_t initialCapacity)
    : mSize(0), mMinCapacity(RoundToNearestPowTwo(initialCapacity)), mStorage(RoundToNearestPowTwo(initialCapacity)) {
    if (Capacity() == 0) {
      mMinCapacity = 2;
      Resize(2);
    }
  }

  ~HashTable() {
  }

  HashTable(const HashTable& rhs)
    : mSize(rhs.mSize), mMinCapacity(rhs.mMinCapacity), mStorage(rhs.mStorage)  {
  }
  
  HashTable(HashTable&& rhs) {
    mSize = rhs.mSize;
    mMinCapacity = rhs.mMinCapacity;
    mStorage = static_cast<Array<HashTable::TableEntry>&&>(rhs.mStorage);
  }

  void operator=(const HashTable& rhs) {
    if (this == &rhs) {
      return;
    }

    mSize = rhs.mSize;
    mMinCapacity = rhs.mMinCapacity;
    mStorage = rhs.mStorage;
  }

  void operator=(HashTable&& rhs) {
    mSize = rhs.mSize;
    mMinCapacity = rhs.mMinCapacity;
    mStorage = static_cast<Array<HashTable::TableEntry>&&>(rhs.mStorage);
  }

  Value& operator[](const Key& key) {
    // Return existing value for key if it exists, whether the value is assigned or not.
    uint32 hashedIndex = HashedIndex(key);
    for (size_t i = 0; i < mSize; ++i) {
      TableEntry& entry = mStorage[(hashedIndex + (i * i)) & (Capacity() - 1)];
      if (entry.occupied && ((Pair<Key, Value>*)entry.kvpBuff)->mKey == key) {
        return ((Pair<Key, Value>*)entry.kvpBuff)->mValue;
      }
      else if (!entry.occupied) {
        entry.occupied = true;
        ((Pair<Key, Value>*)entry.kvpBuff)->mKey = key;
        ++mSize;
        return ((Pair<Key, Value>*)entry.kvpBuff)->mValue;
      }
    }

    if (mSize >= (Capacity() << 1)) {
      const size_t doubledCapacity = (Capacity() >> 1);
      size_t capacity = (doubledCapacity < mMinCapacity) ? mMinCapacity : doubledCapacity;
      Resize(capacity);
    }

    // Should never reach this point but we must return something.
    ZAssert(false);
    return ((Pair<Key, Value>*)mStorage[0].kvpBuff)->mValue;
  }

  bool Add(const Key& key, const Value& value) {
    InsertKeyValue(key, value);

    if (mSize >= (Capacity() << 1)) {
      const size_t doubledCapacity = (Capacity() >> 1);
      size_t capacity = (doubledCapacity < mMinCapacity) ? mMinCapacity : doubledCapacity;
      Resize(capacity);
    }

    return true;
  }

  bool Remove(const Key& key) {
    bool wasRemoved = DeleteKey(key);
    
    if (wasRemoved) {
      const size_t threshold = (Capacity() << 2);
      if (mSize < threshold && mSize > mMinCapacity) {
        const size_t halvedCapacity = (Capacity() << 1);
        size_t capacity = (halvedCapacity < mMinCapacity) ? mMinCapacity : halvedCapacity;
        Resize(capacity);
      }
    }

    return wasRemoved;
  }

  bool HasKey(const Key& key) const {
    uint32 hashedIndex = HashedIndex(key);
    for (size_t i = 0; i < mSize; ++i) {
      const TableEntry& entry = mStorage[(hashedIndex + (i * i)) & (Capacity() - 1)];
      if (entry.occupied && ((Pair<Key, Value>*)entry.kvpBuff)->mKey == key) {
        return true;
      }
      else if (!entry.occupied) {
        return false;
      }
    }

    return false;
  }

  Value GetValue(const Key& key) const {
    uint32 hashedIndex = HashedIndex(key);
    for (size_t i = 0; i < mSize; ++i) {
      const TableEntry& entry = mStorage[(hashedIndex + (i * i)) & (Capacity() - 1)];
      if (entry.occupied && ((Pair<Key, Value>*)entry.kvpBuff)->mKey == key) {
        return ((Pair<Key, Value>*)entry.kvpBuff)->mValue;
      }
      else if (!entry.occupied) {
        break;
      }
    }

    return Value();
  }

  void Resize(size_t size) {
    HashTable tempTable(size);
    for (TableEntry& entry : mStorage) {
      if (entry.occupied) {
        tempTable.Add(((Pair<Key, Value>*)entry.kvpBuff)->mKey, ((Pair<Key, Value>*)entry.kvpBuff)->mValue);
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
    return Iterator(mStorage.begin(), mStorage.end(), false);
  }

  Iterator end() const {
    return Iterator(mStorage.end(), mStorage.end(), false);
  }

  Iterator rbegin() const {
    return Iterator(mStorage.rbegin(), mStorage.rend(), true);
  }

  Iterator rend() const {
    return Iterator(mStorage.rend(), mStorage.rend(), true);
  }

  private:
  size_t mSize;
  size_t mMinCapacity;
  Array<HashTable::TableEntry> mStorage;

  uint32 HashedIndex(const Key& key) const {
    HashFunction hashFunctor;
    uint32 hash = hashFunctor(key);
    return hash & (Capacity() - 1);
  }

  void InsertKeyValue(const Key& key, const Value& value) {
    uint32 hashedIndex = HashedIndex(key);
    for (size_t i = 0; i < Capacity(); ++i) {
      TableEntry& entry = mStorage[(hashedIndex + (i * i)) & (Capacity() - 1)];
      
      // Either insert into an empty slot or update an existing entry.
      if (!entry.occupied) {
        entry.occupied = true;
        new ((Pair<Key, Value>*)(entry.kvpBuff)) Pair<Key, Value>(key, value);
        ++mSize;
        break;
      }
      else if (((Pair<Key, Value>*)entry.kvpBuff)->mKey == key) {
        ((Pair<Key, Value>*)entry.kvpBuff)->mValue = value;
        break;
      }
    }
  }

  bool DeleteKey(const Key& key) {
    uint32 hashedIndex = HashedIndex(key);
    for (size_t i = 0; i < mSize; ++i) {
      TableEntry& entry = mStorage[(hashedIndex + (i * i)) & (Capacity() - 1)];
      if (entry.occupied && ((Pair<Key, Value>*)entry.kvpBuff)->mKey == key) {
        entry.occupied = false;
        ((Pair<Key, Value>*)entry.kvpBuff)->~Pair<Key, Value>();
        --mSize;
        return true;
      }
      else if(!entry.occupied) {
        return false;
      }
    }

    return false;
  }
};

}
