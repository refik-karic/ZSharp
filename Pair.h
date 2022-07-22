#pragma once

namespace ZSharp {

template<typename TKey, typename TValue>
class Pair final {
  public:
  TKey Key;
  TValue Value;
};

}
