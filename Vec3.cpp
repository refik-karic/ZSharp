#include "Vec3.h"

#include <cmath>

namespace ZSharp {

Vec3::Vec3() {
  Clear();
}

Vec3::Vec3(float x, float y, float z) {
  mData[0] = x;
  mData[1] = y;
  mData[2] = z;
}

Vec3::Vec3(const Vec3& copy) {
  *this = copy;
}

Vec3 Vec3::Cross(const Vec3& vec) {
  Vec3 result(
    (mData[1] * vec[2]) - (mData[2] * vec[1]),
    (mData[2] * vec[0]) - (mData[0] * vec[2]),
    (mData[0] * vec[1]) - (mData[1] * vec[0])
  );
  return result;
}

float Vec3::Length() const {
  return sqrtf((*this) * (*this));
}

void Vec3::Normalize() {
  float invSqrt(1.f / Length());
  mData[0] *= invSqrt;
  mData[1] *= invSqrt;
  mData[2] *= invSqrt;
}

void Vec3::Homogenize(std::size_t element) {
  float divisor(mData[element]);

  for (std::size_t i = 0; i <= element; i++) {
    mData[i] /= divisor;
  }
}

void Vec3::Clear() {
  std::memset(mData, 0, sizeof(mData));
}

}
