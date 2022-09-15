#pragma once

#include "Array.h"
#include "HashFunctions.h"
#include "List.h"
#include "Pair.h"
#include "PlatformLogging.h"
#include "PlatformMemory.h"
#include "ZAssert.h"
#include "ZBaseTypes.h"

namespace ZSharp {

template<typename Key, typename Value, typename HashFunction = Hash<Key>>
class HashTable final {
  private:
  struct HashPair : public Pair<Key, Value> {
    HashPair(const Key& key, const Value& value)
      : Pair(key, value) {

    }

    bool operator==(const HashPair& rhs) const {
      return mKey == rhs.mKey;
    }
  };

  public:
  HashTable()
    : mSize(0), mStorage(4096) {

  }

  HashTable(size_t initialCapacity)
    : mSize(0), mStorage(initialCapacity) {

  }

  ~HashTable() {

  }

  HashTable(const HashTable& rhs) = delete;
  HashTable(const HashTable&& rhs) = delete;

  Value& operator[](const Key& key) {
    HashFunction hashFunctor;
    uint32 hash = hashFunctor(key);
    size_t index = hash % Capacity();
    List<HashPair>& bucket = mStorage[index];

    {
      Value dummy{};
      HashPair pair(key, dummy);
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
    HashFunction hashFunctor;
    uint32 hash = hashFunctor(key);
    size_t index = hash % Capacity();
    InsertKeyValue(index, key, value);
    return true;
  }

  bool Remove(const Key& key) {
    HashFunction hashFunctor;
    uint32 hash = hashFunctor(key);
    size_t index = hash % Capacity();
    return DeleteKey(index, key);
  }

  bool HasKey(const Key& key) const {
    HashFunction hashFunctor;
    uint32 hash = hashFunctor(key);
    size_t index = hash % Capacity();
    const List<HashPair>& bucket = mStorage[index];
    Value dummy{};
    HashPair pair(key, dummy);
    return bucket.Contains(pair);
  }

  size_t Size() const {
    return mSize;
  }

  size_t Capacity() const {
    return mStorage.Size();
  }

  private:
  size_t mSize;
  Array<List<HashPair>> mStorage;

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
    Value dummy{};
    HashPair pair(key, dummy);
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
