#pragma once

#include <cstddef>

#include "Vec3.h"

namespace ZSharp {

class Vec4 final {
  public:
  Vec4();

  Vec4(float x, float y, float z, float w);

  Vec4(const Vec4& copy);

  Vec4(const Vec3& copy);

  void operator=(const Vec3& vector) {
    std::memcpy(mData, *vector, sizeof(mData));
  }

  void operator=(const Vec4& vector) {
    if (this == &vector) {
      return;
    }
    
    std::memcpy(mData, *vector, sizeof(mData));
  }

  bool operator==(const Vec4& vector) const {
    if (this == &vector) {
      return true;
    }

    return std::memcmp(mData, *vector, sizeof(mData)) == 0;
  }

  float* operator*() {
    return &mData[0];
  }

  const float* operator*() const {
    return &mData[0];
  }

  float operator[](const std::size_t index) const {
    return mData[index];
  }

  float& operator[](const std::size_t index) {
    return mData[index];
  }

  Vec4 operator+(const Vec4& vector) const {
    Vec4 result(
      mData[0] + vector[0],
      mData[1] + vector[1],
      mData[2] + vector[2],
      mData[3] + vector[3]
    );
    return result;
  }

  Vec4 operator-(const Vec4& vector) const {
    Vec4 result(
      mData[0] - vector[0],
      mData[1] - vector[1],
      mData[2] - vector[2],
      mData[3] - vector[3]
    );
    return result;
  }

  Vec4 operator*(float scalar) const {
    Vec4 result(
      mData[0] * scalar,
      mData[1] * scalar,
      mData[2] * scalar,
      mData[3] * scalar
    );
    return result;
  }

  float operator*(const Vec4& vector) {
    float result = (mData[0] * vector[0]);
    result += (mData[1] * vector[1]);
    result += (mData[2] * vector[2]);
    result += (mData[3] * vector[3]);
    return result;
  }

  float operator*(const Vec4& vector) const {
    float result = (mData[0] * vector[0]);
    result += (mData[1] * vector[1]);
    result += (mData[2] * vector[2]);
    result += (mData[3] * vector[3]);
    return result;
  }

  static float Length(const Vec4& vector);

  static void Normalize(Vec4 vector);

  static void Homogenize(Vec4& vector, std::size_t element);

  static void Clear(Vec4& vector);

  private:
  static const std::size_t Elements = 4;
  float mData[Elements];
};

}
