#pragma once

namespace ZSharp {

template<typename TKey, typename TValue>
class Pair {
  public:
  Pair() = default;

  Pair(const TKey& key) 
    : mKey(key) {

  }

  Pair(const TKey& key, const TValue& value)
  : mKey(key), mValue(value) {

  }

  bool operator==(const Pair& rhs) const {
    return (mKey == rhs.mKey) && (mValue == rhs.mValue);
  }

  TKey mKey;
  TValue mValue;
};

}
