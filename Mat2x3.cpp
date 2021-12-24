#include "Mat2x3.h"

#include <cmath>

namespace ZSharp {
Mat2x3::Mat2x3() {
}

Mat2x3::Mat2x3(const Mat2x3& copy) {
  *this = copy;
}

void Mat2x3::operator=(const Mat2x3& matrix) {
  if (this == &matrix) {
    return;
  }

  mData[0] = matrix[0];
  mData[1] = matrix[1];
}

Vec3& Mat2x3::operator[](size_t index) {
  return mData[index];
}

const Vec3& Mat2x3::operator[](size_t index) const {
  return mData[index];
}

Mat2x3 Mat2x3::operator*(float scalar) const {
  Mat2x3 result;

  result[0] = mData[0] * scalar;
  result[1] = mData[1] * scalar;

  return result;
}

void Mat2x3::Identity() {
  Clear();
  mData[0][0] = 1.f;
  mData[1][1] = 1.f;
}

void Mat2x3::Clear() {
  mData[0].Clear();
  mData[1].Clear();
}

Vec3 Mat2x3::ApplyTransform(const Vec3& domain) const {
  Vec3 codomainResult(
    domain * mData[0],
    domain * mData[1],
    0.f
  );
  return codomainResult;
}
}
