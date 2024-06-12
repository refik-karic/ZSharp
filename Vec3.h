#pragma once

#include "ZBaseTypes.h"
#include "ZString.h"
#include "PlatformDefines.h"

namespace ZSharp {

class Vec3 final {
public:
  Vec3();

  Vec3(float* values);

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

  void operator+=(const Vec3& vector);

  void operator*=(float scalar);

  Vec3 operator-(const Vec3& vector) const;

  Vec3 operator-() const;

  Vec3 operator*(float scalar) const;

  float operator*(const Vec3& vector) const;

  Vec3 Cross(const Vec3& vec) const;

  Vec3 TripleCross(const Vec3& a, const Vec3& b) const;

  Vec3 Parametric(const Vec3& rhs, float t) const;

  float Length() const;

  void Normalize();

  void Homogenize();

  void Clear();

  String ToString() const;

private:
  float mData[3];
};

}
