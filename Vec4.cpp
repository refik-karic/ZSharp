#include "Vec4.h"

#include <cmath>

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
  const float invSqrt = 1.f / Length();
#ifdef FORCE_SSE
  aligned_sse128mulfloat(mData, invSqrt, mData);
#else
  mData[0] *= invSqrt;
  mData[1] *= invSqrt;
  mData[2] *= invSqrt;
  mData[3] *= invSqrt;
#endif
}

void Vec4::Homogenize() {
  const float invDivisor = 1.f / mData[3];
#ifdef FORCE_SSE
  aligned_sse128mulfloat(mData, invDivisor, mData);
#else
  mData[0] *= invDivisor;
  mData[1] *= invDivisor;
  mData[2] *= invDivisor;
  mData[3] *= invDivisor;
#endif
}

void Vec4::Clear() {
  std::memset(mData, 0, sizeof(mData));
}

}
