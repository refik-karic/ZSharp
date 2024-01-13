#include "Triangle.h"

namespace ZSharp {
Triangle::Triangle() {
}

Triangle::Triangle(int32 p1, int32 p2, int32 p3)
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

int32 Triangle::operator[](int32 index) const {
  return mIndicies[index];
}

int32& Triangle::operator[](int32 index) {
  return mIndicies[index];
}

const int32* Triangle::GetData() {
  return mIndicies;
}

const int32* Triangle::GetData() const {
  return mIndicies;
}
}
