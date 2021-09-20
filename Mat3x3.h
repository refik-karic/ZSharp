#pragma once

#include <cstddef>

#include "Vec3.h"

namespace ZSharp {

class Mat3x3 {
  public:
  Mat3x3();

  Mat3x3(const Mat3x3& copy);

  void operator=(const Mat3x3& copy) {
    if (this == &copy) {
      return;
    }

    for (std::size_t row = 0; row < Rows; row++) {
      mData[row] = copy[row];
    }
  }

  Vec3& operator[](std::size_t index) {
    return mData[index];
  }

  const Vec3& operator[](std::size_t index) const {
    return mData[index];
  }

  Mat3x3 operator*(float scalar) {
    Mat3x3 result;

    for (std::size_t row = 0; row < Rows; row++) {
      result[row] = mData[row] * scalar;
    }

    return result;
  }

  Mat3x3 operator*(const Mat3x3& matrix) {
    Mat3x3 result;

    Mat3x3 rhsTranspose(matrix.Transpose());

    for (std::size_t row = 0; row < Rows; row++) {
      for (std::size_t col = 0; col < Columns; col++) {
        result[row][col] = mData[row] * rhsTranspose[col];
      }
    }

    return result;
  }

  void Identity();

  void Clear();

  Mat3x3 Transpose() const;

  private:
  static const std::size_t Rows = 3;
  static const std::size_t Columns = 3;
  Vec3 mData[Rows];
};

}
