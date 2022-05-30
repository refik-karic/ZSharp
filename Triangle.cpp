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

void Triangle::operator=(const Triangle& rhs) {
  if (this == &rhs) {
    return;
  }

  mIndicies[0] = rhs[0];
  mIndicies[1] = rhs[1];
  mIndicies[2] = rhs[2];
}

size_t Triangle::operator[](size_t index) const {
  return mIndicies[index];
}

size_t& Triangle::operator[](size_t index) {
  return mIndicies[index];
}

const size_t* Triangle::GetData() {
  return mIndicies;
}

const size_t* Triangle::GetData() const {
  return mIndicies;
}
}
