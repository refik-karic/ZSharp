#pragma once

#include <cstddef>

#include "UtilMath.h"

namespace ZSharp {

template<typename T>
class Vec3 final {
  public:
  Vec3() {
    Vec3<T>::Clear(*this);
  }

  Vec3(T x, T y, T z)
  {
    mData[0] = x;
    mData[1] = y;
    mData[2] = z;
  }

  Vec3(const Vec3<T>& copy) {
    *this = copy;
  }

  void operator=(const Vec3<T>& vector) {
    if (this == &vector) {
      return;
    }

    std::memcpy(mData, *vector, sizeof(mData));
  }

  bool operator==(const Vec3<T>& vector) const {
    if (this == &vector) {
      return;
    }

    return std::memcmp(mData, *vector, sizeof(mData)) == 0;
  }

  T* operator*() {
    return &mData[0];
  }

  const T* operator*() const {
    return &mData[0];
  }

  T operator[](const std::size_t index) const {
    return mData[index];
  }

  T& operator[](const std::size_t index) {
    return mData[index];
  }

  Vec3<T> operator+(const Vec3<T>& vector) const {
    Vec3<T> result(
      mData[0] + vector[0], 
      mData[1] + vector[1], 
      mData[2] + vector[2]
    );
    return result;
  }

  Vec3<T> operator-(const Vec3<T>& vector) const {
    Vec3<T> result(
      mData[0] - vector[0], 
      mData[1] - vector[1], 
      mData[2] - vector[2]
    );
    return result;
  }

  Vec3<T> operator*(T scalar) const {
    Vec3<T> result(mData[0] * scalar, mData[1] * scalar, mData[2] * scalar);
    return result;
  }

  T operator*(const Vec3<T>& vector) {
    T result = (mData[0] * vector[0]);
    result += (mData[1] * vector[1]);
    result += (mData[2] * vector[2]);
    return result;
  }

  T operator*(const Vec3<T>& vector) const {
    T result = (mData[0] * vector[0]);
    result += (mData[1] * vector[1]);
    result += (mData[2] * vector[2]);
    return result;
  }

  static Vec3<T> Cross(const Vec3<T>& v1, const Vec3<T>& v2) {
    Vec3<T> result(
      (v1[1] * v2[2]) - (v1[2] * v2[1]),
      (v1[2] * v2[0]) - (v1[0] * v2[2]),
      (v1[0] * v2[1]) - (v1[1] * v2[0])
    );
    return result;
  }

  static T Length(const Vec3<T>& vector) {
    return NewtonRaphsonSqrt(vector * vector);
  }

  static void Normalize(Vec3<T>& vector) {
    T invSqrt(1 / Length(vector));
    vector[0] *= invSqrt;
    vector[1] *= invSqrt;
    vector[2] *= invSqrt;
  }

  static void Homogenize(Vec3<T>& vector, std::size_t element) {
    T divisor(vector[element]);

    for (std::size_t i = 0; i <= element; i++) {
      vector[i] /= divisor;
    }
  }

  static void Clear(Vec3<T>& vector) {
    std::memset(*vector, 0, sizeof(T) * Elements);
  }

  private:
  static const std::size_t Elements = 3;
  T mData[Elements];
};

}
