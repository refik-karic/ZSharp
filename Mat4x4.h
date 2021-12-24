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

  void operator=(const Mat4x4& matrix);

  Vec4& operator[](size_t index);

  const Vec4& operator[](size_t index) const;

  Mat4x4 operator*(float scalar);

  Mat4x4 operator*(const Mat4x4& matrix);

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
