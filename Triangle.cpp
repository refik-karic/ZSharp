#include "Triangle.h"

namespace ZSharp {
Triangle::Triangle() {
}

Triangle::Triangle(std::size_t p1, std::size_t p2, std::size_t p3)
  : mIndicies{ p1, p2, p3 } {
}

Triangle::Triangle(const Triangle& copy) {
  *this = copy;
}

const std::size_t* Triangle::GetData() {
  return mIndicies;
}

const std::size_t* Triangle::GetData() const {
  return mIndicies;
}
}
