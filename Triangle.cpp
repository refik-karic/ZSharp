#include "Triangle.h"

namespace ZSharp {
Triangle::Triangle() {
}

Triangle::Triangle(size_t p1, size_t p2, size_t p3)
  : mIndicies{ p1, p2, p3 } {
}

Triangle::Triangle(const Triangle& copy) {
  *this = copy;
}

const size_t* Triangle::GetData() {
  return mIndicies;
}

const size_t* Triangle::GetData() const {
  return mIndicies;
}
}
