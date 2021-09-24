#pragma once

#include <cstddef>

#include "Vec3.h"
#include "Vec4.h"

namespace ZSharp {

class Mat4x4 final {
  public:
  enum class Axis {
    X,
    Y,
    Z
  };

  Mat4x4();

  Mat4x4(const Mat4x4& copy);

  void operator=(const Mat4x4& matrix) {
    if (this == &matrix) {
      return;
    }

    for (size_t row = 0; row < Rows; row++) {
      mData[row] = matrix[row];
    }
  }

  Vec4& operator[](size_t index) {
    return mData[index];
  }

  const Vec4& operator[](size_t index) const {
    return mData[index];
  }

  Mat4x4 operator*(float scalar) {
    Mat4x4 result;

    for (size_t row = 0; row < Rows; row++) {
      result[row] = mData[row] * scalar;
    }

    return result;
  }

  Mat4x4 operator*(const Mat4x4& matrix) {
    Mat4x4 result;

    Mat4x4 rhsTranspose(matrix.Transpose());

    for (size_t row = 0; row < Rows; row++) {
      for (size_t col = 0; col < Columns; col++) {
        result[row][col] = mData[row] * rhsTranspose[col];
      }
    }

    return result;
  }

  void Identity();
  
  void Clear();

  Mat4x4 Transpose() const;

  void SetTranslation(const Vec3& translation);

  void SetTranslation(const Vec4& translation);

  void SetScale(const Vec4& scale);

  void SetRotation(float angle, Axis axis);

  Vec4 ApplyTransform(const Vec4& domain) const;

  private:
  static const size_t Rows = 4;
  static const size_t Columns = 4;
  Vec4 mData[Rows];
};

}
