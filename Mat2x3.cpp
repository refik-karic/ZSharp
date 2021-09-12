#include "Mat2x3.h"

#include <cmath>

namespace ZSharp {
Mat2x3::Mat2x3() {
}

Mat2x3::Mat2x3(const Mat2x3& copy) {
  *this = copy;
}

void Mat2x3::Identity() {
  for (std::size_t row = 0; row < Rows; row++) {
    for (std::size_t col = 0; col < Columns; col++) {
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
  for (std::size_t row = 0; row < Rows; row++) {
    mData[row].Clear();
  }
}

Mat2x3 Mat2x3::Transpose() const {
  Mat2x3 result;

  for (std::size_t row = 0; row < Rows; row++) {
    for (std::size_t col = 0; col < Columns; col++) {
      result[row][col] = mData[col][row];
    }
  }

  return result;
}

void Mat2x3::SetTranslation(const Vec3& translation) {
  std::size_t lastColumn = Columns - 1;
  for (std::size_t row = 0; row < Columns; row++) {
    mData[row][lastColumn] = translation[row];
  }
}

void Mat2x3::SetScale(const Vec3& scale) {
  for (std::size_t row = 0; row < Columns; row++) {
    mData[row][row] = scale[row];
  }
}

void Mat2x3::SetRotation(float angle, Axis axis) {
  switch (axis) {
  case Axis::Z:
    mData[0][0] = cosf(angle);
    mData[0][1] = -1.f * sinf(angle);
    mData[1][0] = sinf(angle);
    mData[1][1] = cosf(angle);
    mData[2][2] = 1.f;
    mData[3][3] = 1.f;
    break;
  case Axis::X:
    mData[0][0] = 1.f;
    mData[1][1] = cosf(angle);
    mData[1][2] = -1.f * sinf(angle);
    mData[2][1] = sinf(angle);
    mData[2][2] = cosf(angle);
    mData[3][3] = 1.f;
    break;
  case Axis::Y:
    mData[0][0] = cosf(angle);
    mData[0][2] = sinf(angle);
    mData[1][1] = 1.f;
    mData[2][0] = -1.f * sinf(angle);
    mData[2][2] = cosf(angle);
    mData[3][3] = 1.f;
    break;
  }
}

Vec3 Mat2x3::ApplyTransform(const Vec3& domain) const {
  Vec3 codomainResult;

  for (std::size_t row = 0; row < Rows; row++) {
    codomainResult[row] = domain * mData[row];
  }

  return codomainResult;
}
}
