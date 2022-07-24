#pragma once

namespace ZSharp {

template<typename TKey, typename TValue>
class Pair final {
  public:
  Pair() = default;

  Pair(TKey key, TValue value)
  : mKey(key), mValue(value) {

  }

  TKey mKey;
  TValue mValue;
};

}
