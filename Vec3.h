#pragma once

#include <cstddef>
#include <cstring>

namespace ZSharp {

class Vec3 final {
public:
  Vec3();

  Vec3(float x, float y, float z);

  Vec3(const Vec3& copy);

  void operator=(const Vec3& vector) {
    if (this == &vector) {
      return;
    }

    std::memcpy(mData, *vector, sizeof(mData));
  }

  bool operator==(const Vec3& vector) const {
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

  float operator[](const size_t index) const {
    return mData[index];
  }

  float& operator[](const size_t index) {
    return mData[index];
  }

  Vec3 operator+(const Vec3& vector) const {
    Vec3 result(
      mData[0] + vector[0],
      mData[1] + vector[1],
      mData[2] + vector[2]
    );
    return result;
  }

  Vec3 operator-(const Vec3& vector) const {
    Vec3 result(
      mData[0] - vector[0],
      mData[1] - vector[1],
      mData[2] - vector[2]
    );
    return result;
  }

  Vec3 operator-() const {
    Vec3 result(
      -mData[0],
      -mData[1],
      -mData[2]
    );
    return result;
  }

  Vec3 operator*(float scalar) const {
    Vec3 result(mData[0] * scalar, mData[1] * scalar, mData[2] * scalar);
    return result;
  }

  float operator*(const Vec3& vector) const {
    return (mData[0] * vector[0]) + (mData[1] * vector[1]) + (mData[2] * vector[2]);
  }

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
