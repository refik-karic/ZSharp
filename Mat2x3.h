#pragma once

#include <cstddef>

#include "Vec3.h"

namespace ZSharp {

class Mat2x3 final {
  public:

  enum class Axis {
    X,
    Y,
    Z
  };

  Mat2x3();

  Mat2x3(const Mat2x3& copy);

  void operator=(const Mat2x3& matrix) {
    if (this == &matrix) {
      return;
    }

    for (std::size_t row = 0; row < Rows; row++) {
      mData[row] = matrix[row];
    }
  }

  Vec3& operator[](std::size_t index) {
    return mData[index];
  }

  const Vec3& operator[](std::size_t index) const {
    return mData[index];
  }

  Mat2x3 operator*(float scalar) {
    Mat2x3 result;

    for (std::size_t row = 0; row < Rows; row++) {
      result[row] = mData[row] * scalar;
    }

    return result;
  }

  Mat2x3 operator*(const Mat2x3& matrix) {
    Mat2x3 result;

    Mat2x3 rhsTranspose(matrix.Transpose());

    for (std::size_t row = 0; row < Rows; row++) {
      for (std::size_t col = 0; col < Columns; col++) {
        result[row][col] = mData[row] * rhsTranspose[col];
      }
    }

    return result;
  }

  void Identity();

  void Clear();

  Mat2x3 Transpose() const;

  Vec3 ApplyTransform(const Vec3& domain) const;

  private:
  static const std::size_t Rows = 2;
  static const std::size_t Columns = 3;
  Vec3 mData[Rows];
};

}
