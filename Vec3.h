#pragma once

#include "ZBaseTypes.h"

namespace ZSharp {

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

private:
  static const size_t Elements = 3;
  float mData[Elements];
};

}
