#pragma once

#include "ZBaseTypes.h"

#include "Vec3.h"
#include "Vec4.h"

#include "Quaternion.h"

namespace ZSharp {

class Mat4x4 final {
  public:

  Mat4x4();

  Mat4x4(const Vec4& a, const Vec4& b, const Vec4& c, const Vec4& d);

  Mat4x4(const Mat4x4& copy);

  void operator=(const Mat4x4& matrix);

  Vec4& operator[](size_t index);

  const Vec4& operator[](size_t index) const;

  float* operator*();

  const float* operator*() const;

  Mat4x4 operator*(float scalar);

  Mat4x4 operator*(const Mat4x4& matrix) const;

  Vec4 operator*(const Vec4& domain) const;

  Vec4& operator*=(Vec4& domain) const;

  Mat4x4& operator*=(const Mat4x4& matrix);

  void Identity();
  
  void Clear();

  Mat4x4 Transpose() const;

  void SetTranslation(const Vec3& translation);

  void SetTranslation(const Vec4& translation);

  void SetScale(const Vec4& scale);

  void SetRotation(const Quaternion& quat);

  private:
  Vec4 mData[4];
};

}
