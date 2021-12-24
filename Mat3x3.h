#pragma once

#include <cstddef>

#include "Vec3.h"

namespace ZSharp {

class Mat3x3 {
  public:
  Mat3x3();

  Mat3x3(const Mat3x3& copy);

  void operator=(const Mat3x3& copy);

  Vec3& operator[](size_t index);

  const Vec3& operator[](size_t index) const;

  Mat3x3 operator*(float scalar);

  Mat3x3 operator*(const Mat3x3& matrix);

  void Identity();

  void Clear();

  Mat3x3 Transpose() const;

  private:
  static const size_t Rows = 3;
  static const size_t Columns = 3;
  Vec3 mData[Rows];
};

}
