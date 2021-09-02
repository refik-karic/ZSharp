#include "Mat4x4.h"

#include <cmath>

namespace ZSharp {

Mat4x4::Mat4x4() {
}

Mat4x4::Mat4x4(const Mat4x4& copy) {
  *this = copy;
}

void Mat4x4::Identity() {
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

void Mat4x4::Clear() {
  for (std::size_t row = 0; row < Rows; row++) {
    mData[row].Clear();
  }
}

Mat4x4 Mat4x4::Transpose() const {
  Mat4x4 result;

  for (std::size_t row = 0; row < Rows; row++) {
    for (std::size_t col = 0; col < Columns; col++) {
      result[row][col] = mData[col][row];
    }
  }

  return result;
}

void Mat4x4::SetTranslation(const Vec3& translation) {
  std::size_t lastColumn = Columns - 1;
  for (std::size_t row = 0; row < 3; row++) {
    mData[row][lastColumn] = translation[row];
  }
}

void Mat4x4::SetTranslation(const Vec4& translation) {
  std::size_t lastColumn = Columns - 1;
  for (std::size_t row = 0; row < Columns; row++) {
    mData[row][lastColumn] = translation[row];
  }
}

void Mat4x4::SetScale(const Vec4& scale) {
  for (std::size_t row = 0; row < Columns; row++) {
    mData[row][row] = scale[row];
  }
}

void Mat4x4::SetRotation(float angle, Axis axis) {
#pragma warning(disable: 4244)

  switch (axis) {
  case Axis::Z:
  case Axis::TWO_DIMENSIONS:
    mData[0][0] = cosf(angle);
    mData[0][1] = -1.f * sinf(angle);
    mData[1][0] = sinf(angle);
    mData[1][1] = cosf(angle);
    mData[2][2] = 1.f;

    if (axis == Axis::Z) {
      mData[3][3] = 1.f;
    }
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

#pragma warning(default: 4244)
}

Vec4 Mat4x4::ApplyTransform(const Vec4& domain) const {
  Vec4 codomainResult;

  for (std::size_t row = 0; row < Rows; row++) {
    codomainResult[row] = domain * mData[row];
  }

  return codomainResult;
}

}
