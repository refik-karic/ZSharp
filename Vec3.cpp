#include "Vec3.h"

#include <cmath>

namespace ZSharp {

Vec3::Vec3() {
  Clear();
}

Vec3::Vec3(float x, float y, float z)
  : mData{x, y, z} {
}

Vec3::Vec3(const Vec3& copy) {
  *this = copy;
}

Vec3 Vec3::Cross(const Vec3& vec) const {
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
  const float invSqrt(1.f / Length());
  mData[0] *= invSqrt;
  mData[1] *= invSqrt;
  mData[2] *= invSqrt;
}

void Vec3::Homogenize() {
  const float invDivisor = 1.f / mData[2];
  mData[0] *= invDivisor;
  mData[1] *= invDivisor;
  mData[2] *= invDivisor;
}

void Vec3::Clear() {
  memset(mData, 0, sizeof(mData));
}

}
