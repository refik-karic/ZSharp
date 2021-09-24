#pragma once

#include <cstddef>

#include "Constants.h"

namespace ZSharp {

class Triangle final {
  public:

  Triangle();

  Triangle(size_t p1, size_t p2, size_t p3);

  Triangle(const Triangle& copy);

  void operator=(const Triangle& rhs) {
    if (this == &rhs) {
      return;
    }

    mIndicies[0] = rhs[0];
    mIndicies[1] = rhs[1];
    mIndicies[2] = rhs[2];
  }

  size_t operator[](size_t index) const {
    return mIndicies[index];
  }

  size_t& operator[](size_t index) {
    return mIndicies[index];
  }

  const size_t* GetData();

  const size_t* GetData() const;

  private:
  size_t mIndicies[TRI_VERTS] = {0, 0, 0};
};

}
