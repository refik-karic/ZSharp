#include "Mat2x3.h"

#include <cmath>

namespace ZSharp {
Mat2x3::Mat2x3() {
}

Mat2x3::Mat2x3(const Mat2x3& copy) {
  *this = copy;
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
  Vec3 codomainResult(
    domain * mData[0],
    domain * mData[1],
    0.f
  );
  return codomainResult;
}
}
