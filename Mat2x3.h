#pragma once

#include "ZBaseTypes.h"

#include "Vec3.h"

namespace ZSharp {

class Mat2x3 final {
  public:

  Mat2x3();

  Mat2x3(const Mat2x3& copy);

  void operator=(const Mat2x3& matrix);

  Vec3& operator[](size_t index);

  const Vec3& operator[](size_t index) const;

  Mat2x3 operator*(float scalar) const;

  Mat2x3& operator*=(float scalar);

  void Identity();

  void Clear();

  Vec3 ApplyTransform(const Vec3& domain) const;

  private:
  Vec3 mData[2];
};

}
