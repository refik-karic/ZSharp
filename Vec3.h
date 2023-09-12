#pragma once

#include "ZBaseTypes.h"
#include "ZString.h"
#include "PlatformDefines.h"

namespace ZSharp {

FORCE_INLINE float Dot3(const float lhs[3], const float rhs[3]) {
  return (lhs[0] * rhs[0]) +
    (lhs[1] * rhs[1]) +
    (lhs[2] * rhs[2]);
}

class Vec3 final {
public:
  Vec3();

  Vec3(float value);

  Vec3(float x, float y, float z);

  Vec3(const Vec3& copy);

  void operator=(const Vec3& vector);

  bool operator==(const Vec3& vector) const;

  float* operator*();

  const float* operator*() const;

  float operator[](const size_t index) const;

  float& operator[](const size_t index);

  Vec3 operator+(const Vec3& vector) const;

  Vec3 operator-(const Vec3& vector) const;

  Vec3 operator-() const;

  Vec3 operator*(float scalar) const;

  float operator*(const Vec3& vector) const;

  Vec3 Cross(const Vec3& vec) const;

  float Length() const;

  void Normalize();

  void Homogenize();

  void Clear();

  String ToString() const;

private:
  float mData[3];
};

}
