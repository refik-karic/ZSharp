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
  private:
  struct HashPair : public Pair<Key, Value> {
    HashPair(const Key& key) 
      : Pair(key) {

    }

    HashPair(const Key& key, const Value& value)
      : Pair(key, value) {

    }

    bool operator==(const HashPair& rhs) const {
      return mKey == rhs.mKey;
    }
  };

  public:
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
  
  HashTable(const HashTable&& rhs) = delete;

  Value& operator[](const Key& key) {
    List<HashPair>& bucket = mStorage[HashedIndex(key)];
    {
      HashPair pair(key);
      if (!bucket.Contains(pair)) {
        bucket.Add(pair);
      }
    }

    Value& matchedValue = (*(bucket.begin())).mValue;
    for (HashPair& pair : bucket) {
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
    const List<HashPair>& bucket = mStorage[HashedIndex(key)];
    HashPair pair(key);
    return bucket.Contains(pair);
  }

  void Resize(size_t size) {
    HashTable tempTable(size);
    for (List<HashPair>& bucket : mStorage) {
      for (HashPair& hashPair : bucket) {
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

  private:
  const size_t mMinCapacity = 4096;
  size_t mSize;
  Array<List<HashPair>> mStorage;

  uint32 HashedIndex(const Key& key) const {
    HashFunction hashFunctor;
    uint32 hash = hashFunctor(key);
    return hash % Capacity();
  }

  void InsertKeyValue(size_t index, const Key& key, const Value& value) {
    List<HashPair>& bucket = mStorage[index];
    if (bucket.Size() == 0) {
      HashPair pair(key, value);
      bucket.Add(pair);
      ++mSize;
    }
    else {
      bool foundMatch = false;
      for (HashPair& pair : bucket) {
        if (pair.mKey == key) {
          pair.mValue = value;
          foundMatch = true;
          break;
        }
      }

      if (!foundMatch) {
        HashPair pair(key, value);
        bucket.Add(pair);
        ++mSize;
      }
    }
  }

  bool DeleteKey(size_t index, const Key& key) {
    List<HashPair>& bucket = mStorage[index];
    HashPair pair(key);
    if (bucket.Remove(pair)) {
      --mSize;
      return true;
    }
    else {
      return false;
    }
  }
};

}
