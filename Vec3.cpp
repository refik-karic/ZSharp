#include "Vec3.h"

#include <cmath>

namespace ZSharp {

Vec3::Vec3() {
  Vec3::Clear(*this);
}

Vec3::Vec3(float x, float y, float z) {
  mData[0] = x;
  mData[1] = y;
  mData[2] = z;
}

Vec3::Vec3(const Vec3& copy) {
  *this = copy;
}

Vec3 Vec3::Cross(const Vec3& v1, const Vec3& v2) {
  Vec3 result(
    (v1[1] * v2[2]) - (v1[2] * v2[1]),
    (v1[2] * v2[0]) - (v1[0] * v2[2]),
    (v1[0] * v2[1]) - (v1[1] * v2[0])
  );
  return result;
}

float Vec3::Length(const Vec3& vector) {
  return sqrtf(vector * vector);
}

void Vec3::Normalize(Vec3& vector) {
  float invSqrt(1.f / Length(vector));
  vector[0] *= invSqrt;
  vector[1] *= invSqrt;
  vector[2] *= invSqrt;
}

void Vec3::Homogenize(Vec3& vector, std::size_t element) {
  float divisor(vector[element]);

  for (std::size_t i = 0; i <= element; i++) {
    vector[i] /= divisor;
  }
}

void Vec3::Clear(Vec3& vector) {
  std::memset(*vector, 0, sizeof(Elements));
}

}
