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
    
    mData[0] = matrix[0];
    mData[1] = matrix[1];
  }

  Vec3& operator[](size_t index) {
    return mData[index];
  }

  const Vec3& operator[](size_t index) const {
    return mData[index];
  }

  Mat2x3 operator*(float scalar) const {
    Mat2x3 result;

    result[0] = mData[0] * scalar;
    result[1] = mData[1] * scalar;

    return result;
  }

  void Identity();

  void Clear();

  Vec3 ApplyTransform(const Vec3& domain) const;

  private:
  static const size_t Rows = 2;
  static const size_t Columns = 3;
  Vec3 mData[Rows];
};

}
