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

    mData[0] = matrix[0];
    mData[1] = matrix[1];
    mData[2] = matrix[2];
    mData[3] = matrix[3];
  }

  Vec4& operator[](size_t index) {
    return mData[index];
  }

  const Vec4& operator[](size_t index) const {
    return mData[index];
  }

  Mat4x4 operator*(float scalar) {
    Mat4x4 result;

    result[0] = mData[0] * scalar;
    result[1] = mData[1] * scalar;
    result[2] = mData[2] * scalar;
    result[3] = mData[3] * scalar;

    return result;
  }

  Mat4x4 operator*(const Mat4x4& matrix) {
    Mat4x4 result;

    const Mat4x4 rhsTranspose(matrix.Transpose());

    result[0][0] = mData[0] * rhsTranspose[0];
    result[0][1] = mData[0] * rhsTranspose[1];
    result[0][2] = mData[0] * rhsTranspose[2];
    result[0][3] = mData[0] * rhsTranspose[3];

    result[1][0] = mData[1] * rhsTranspose[0];
    result[1][1] = mData[1] * rhsTranspose[1];
    result[1][2] = mData[1] * rhsTranspose[2];
    result[1][3] = mData[1] * rhsTranspose[3];

    result[2][0] = mData[2] * rhsTranspose[0];
    result[2][1] = mData[2] * rhsTranspose[1];
    result[2][2] = mData[2] * rhsTranspose[2];
    result[2][3] = mData[2] * rhsTranspose[3];

    result[3][0] = mData[3] * rhsTranspose[0];
    result[3][1] = mData[3] * rhsTranspose[1];
    result[3][2] = mData[3] * rhsTranspose[2];
    result[3][3] = mData[3] * rhsTranspose[3];

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
