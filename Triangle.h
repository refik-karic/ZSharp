#pragma once

#include <cstddef>

#include "Constants.h"

namespace ZSharp {

template<typename T>
class Triangle final {
  public:

  Triangle() {}

  Triangle(std::size_t p1, std::size_t p2, std::size_t p3) :
    mIndicies{p1, p2, p3}
  { }

  Triangle(const Triangle<T>& copy) {
    if (this == &copy) {
      return;
    }

    *this = copy;
  }

  void operator=(const Triangle<T>& rhs) {
    if (this == &rhs) {
      return;
    }

    mIndicies[0] = rhs[0];
    mIndicies[1] = rhs[1];
    mIndicies[2] = rhs[2];
  }

  std::size_t operator[](std::size_t index) const {
    return mIndicies[index];
  }

  std::size_t& operator[](std::size_t index) {
    return mIndicies[index];
  }

  const std::size_t* GetData() {
    return mIndicies;
  }

  const std::size_t* GetData() const {
    return mIndicies;
  }

  private:
  std::size_t mIndicies[Constants::TRI_VERTS] = {0, 0, 0};
};

}
