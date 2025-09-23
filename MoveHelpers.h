#pragma once

#include <type_traits>

namespace ZSharp {

template<typename T>
constexpr typename std::remove_reference<T>::type&& Move(T&& arg) {
  return static_cast<typename std::remove_reference<T>::type&&>(arg);
}

template <typename T>
constexpr T&& Forward(typename std::remove_reference<T>::type& t) noexcept {
  return static_cast<T&&>(t);
}

template <typename T>
constexpr T&& Forward(typename std::remove_reference<T>::type&& t) noexcept {
  static_assert(!std::is_lvalue_reference<T>::value, "Can't forward rvalue as lvalue.");
  return static_cast<T&&>(t);
}

}
