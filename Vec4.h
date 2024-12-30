#pragma once

#include "ZBaseTypes.h"
#include "Vec3.h"
#include "ZString.h"

namespace ZSharp {

class alignas(16) Vec4 final {
  public:
  Vec4();

  Vec4(float* values);

  Vec4(float value);

  Vec4(float x, float y, float z, float w);

  Vec4(const Vec4& copy);

  Vec4(const Vec3& copy);

  Vec4(const Vec3& copy, float w);

  void operator=(const Vec3& vector);

  void operator=(const Vec4& vector);

  bool operator==(const Vec4& vector) const;

  float* operator*();

  const float* operator*() const;

  float operator[](const size_t index) const;

  float& operator[](const size_t index);

  Vec4 operator+(const Vec4& vector) const;

  Vec4 operator-(const Vec4& vector) const;

  Vec4 operator-() const;

  Vec4 operator*(float scalar) const;

  void operator*=(float scalar);

  float operator*(const Vec4& vector) const;

  float Length() const;

  void Normalize();

  void Homogenize();

  void Clear();

  String ToString() const;

  private:
  float mData[4];
};

}
