#include "Vec4.h"

#include <cmath>

#include "UtilMath.h"

namespace ZSharp {

Vec4::Vec4() {
  Clear();
}

Vec4::Vec4(float x, float y, float z, float w) {
  mData[0] = x;
  mData[1] = y;
  mData[2] = z;
  mData[3] = w;
}

Vec4::Vec4(const Vec4& copy) {
  *this = copy;
}

Vec4::Vec4(const Vec3& copy) {
  *this = copy;
}

float Vec4::Length() const {
  return sqrtf((*this) * (*this));
}

void Vec4::Normalize() {
  float invSqrt(1.f / Length());
  mData[0] *= invSqrt;
  mData[1] *= invSqrt;
  mData[2] *= invSqrt;
  mData[3] *= invSqrt;
}

void Vec4::Homogenize(size_t element) {
  float divisor(mData[element]);

  for (size_t i = 0; i <= element; i++) {
    mData[i] /= divisor;
  }
}

void Vec4::Clear() {
  std::memset(mData, 0, sizeof(mData));
}

}
