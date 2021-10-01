#include "Mat2x3.h"

#include <cmath>

namespace ZSharp {
Mat2x3::Mat2x3() {
}

Mat2x3::Mat2x3(const Mat2x3& copy) {
  *this = copy;
}

void Mat2x3::Identity() {
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

void Mat2x3::Clear() {
  for (size_t row = 0; row < Rows; row++) {
    mData[row].Clear();
  }
}

Mat2x3 Mat2x3::Transpose() const {
  Mat2x3 result;

  for (size_t row = 0; row < Rows; row++) {
    for (size_t col = 0; col < Columns; col++) {
      result[row][col] = mData[col][row];
    }
  }

  return result;
}

Vec3 Mat2x3::ApplyTransform(const Vec3& domain) const {
  Vec3 codomainResult;

  codomainResult[0] = domain * mData[0];
  codomainResult[1] = domain * mData[1];
  codomainResult[2] = domain * mData[3];

  return codomainResult;
}
}
