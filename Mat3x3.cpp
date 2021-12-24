#include "Mat3x3.h"

namespace ZSharp {
Mat3x3::Mat3x3() {
}

Mat3x3::Mat3x3(const Mat3x3& copy) {
  *this = copy;
}

void Mat3x3::operator=(const Mat3x3& copy) {
  if (this == &copy) {
    return;
  }

  for (size_t row = 0; row < Rows; row++) {
    mData[row] = copy[row];
  }
}

Vec3& Mat3x3::operator[](size_t index) {
  return mData[index];
}

const Vec3& Mat3x3::operator[](size_t index) const {
  return mData[index];
}

Mat3x3 Mat3x3::operator*(float scalar) {
  Mat3x3 result;

  for (size_t row = 0; row < Rows; row++) {
    result[row] = mData[row] * scalar;
  }

  return result;
}

Mat3x3 Mat3x3::operator*(const Mat3x3& matrix) {
  Mat3x3 result;

  Mat3x3 rhsTranspose(matrix.Transpose());

  for (size_t row = 0; row < Rows; row++) {
    for (size_t col = 0; col < Columns; col++) {
      result[row][col] = mData[row] * rhsTranspose[col];
    }
  }

  return result;
}

void Mat3x3::Identity() {
  for (size_t row = 0; row < Rows; row++) {
    for (size_t col = 0; col < Columns; col++) {
      if (row == col) {
        mData[row][col] = 1.f;
      }
      else {
        mData[row][col] = 0.f;
      }
    }
  }
}

void Mat3x3::Clear() {
  for (size_t row = 0; row < Rows; row++) {
    mData[row].Clear();
  }
}

Mat3x3 Mat3x3::Transpose() const {
  Mat3x3 result;

  for (size_t row = 0; row < Rows; row++) {
    for (size_t col = 0; col < Columns; col++) {
      result[row][col] = mData[col][row];
    }
  }

  return result;
}

}
