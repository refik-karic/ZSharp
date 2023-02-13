#include "Mat3x3.h"

#include <cstring>

namespace ZSharp {
Mat3x3::Mat3x3() {
}

Mat3x3::Mat3x3(const Mat3x3& copy)
  : mData{copy.mData[0], copy.mData[1], copy.mData[2]} {
}

void Mat3x3::operator=(const Mat3x3& copy) {
  if (this == &copy) {
    return;
  }

  memcpy(mData, copy.mData, sizeof(mData));
}

Vec3& Mat3x3::operator[](size_t index) {
  return mData[index];
}

const Vec3& Mat3x3::operator[](size_t index) const {
  return mData[index];
}

Mat3x3 Mat3x3::operator*(float scalar) {
  Mat3x3 result;

  result[0] = mData[0] * scalar;
  result[1] = mData[1] * scalar;
  result[2] = mData[2] * scalar;

  return result;
}

Mat3x3 Mat3x3::operator*(const Mat3x3& matrix) {
  Mat3x3 result;

  const Mat3x3 rhsTranspose(matrix.Transpose());

  result[0][0] = mData[0] * rhsTranspose[0];
  result[0][1] = mData[0] * rhsTranspose[1];
  result[0][2] = mData[0] * rhsTranspose[2];

  result[1][0] = mData[1] * rhsTranspose[0];
  result[1][1] = mData[1] * rhsTranspose[1];
  result[1][2] = mData[1] * rhsTranspose[2];

  result[2][0] = mData[2] * rhsTranspose[0];
  result[2][1] = mData[2] * rhsTranspose[1];
  result[2][2] = mData[2] * rhsTranspose[2];

  return result;
}

void Mat3x3::Identity() {
  Clear();
  mData[0][0] = 1.f;
  mData[1][1] = 1.f;
  mData[2][2] = 1.f;
}

void Mat3x3::Clear() {
  memset(mData, 0, sizeof(mData));
}

Mat3x3 Mat3x3::Transpose() const {
  Mat3x3 result;

  result[0][0] = mData[0][0];
  result[0][1] = mData[1][0];
  result[0][2] = mData[2][0];

  result[1][0] = mData[0][1];
  result[1][1] = mData[1][1];
  result[1][2] = mData[2][1];

  result[2][0] = mData[0][2];
  result[2][1] = mData[1][2];
  result[2][2] = mData[2][2];

  return result;
}

}
