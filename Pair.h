#pragma once

#include "MoveHelpers.h"

namespace ZSharp {

template<typename TKey, typename TValue>
class Pair {
  public:
  Pair() = default;

  Pair(const Pair& rhs) : mKey(rhs.mKey), mValue(rhs.mValue) {}

  Pair(Pair&& rhs) : mKey(Move(rhs.mKey)), mValue(Move(rhs.mValue)) {}

  Pair(const TKey& key) : mKey(key) {}

  Pair(const TKey& key, const TValue& value) : mKey(key), mValue(value) {}

  void operator=(const Pair& rhs) {
    if (this != &rhs) {
      mKey = rhs.mKey;
      mValue = rhs.mValue;
    }
  }

  void operator=(Pair&& rhs) {
    mKey = Move(rhs.mKey);
    mValue = Move(rhs.mValue);
  }

  bool operator==(const Pair& rhs) const {
    return (mKey == rhs.mKey) && (mValue == rhs.mValue);
  }

  TKey mKey;
  TValue mValue;
};

}
