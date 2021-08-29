#include "Vec4.h"

#include <cmath>

namespace ZSharp {

Vec4::Vec4() {
  Vec4::Clear(*this);
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

float Vec4::Length(const Vec4& vector) {
  return sqrtf(vector * vector);
}

void Vec4::Normalize(Vec4 vector) {
  float invSqrt(1.f / Length(vector));
  vector[0] *= invSqrt;
  vector[1] *= invSqrt;
  vector[2] *= invSqrt;
  vector[3] *= invSqrt;
}

void Vec4::Homogenize(Vec4& vector, std::size_t element) {
  float divisor(vector[element]);

  for (std::size_t i = 0; i <= element; i++) {
    vector[i] /= divisor;
  }
}

void Vec4::Clear(Vec4& vector) {
  std::memset(*vector, 0, sizeof(mData));
}

}
