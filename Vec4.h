#pragma once

#include <cstddef>

#include "UtilMath.h"
#include "Vec3.h"

namespace ZSharp {

template<typename T>
class Vec4 final {
  public:
  Vec4() {
    Vec4<T>::Clear(*this);
  }

  Vec4(T x, T y, T z, T w) {
    mData[0] = x;
    mData[1] = y;
    mData[2] = z;
    mData[3] = w;
  }

  Vec4(const Vec4<T>& copy) {
    *this = copy;
  }

  Vec4(const Vec3<T>& copy) {
    *this = copy;
  }

  void operator=(const Vec3<T>& vector) {
    std::memcpy(mData, *vector, sizeof(T) * 3);
  }

  void operator=(const Vec4<T>& vector) {
    if (this == &vector) {
      return;
    }
    
    std::memcpy(mData, *vector, sizeof(mData));
  }

  bool operator==(const Vec4<T>& vector) const {
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

  Vec4<T> operator+(const Vec4<T>& vector) const {
    Vec4<T> result(
      mData[0] + vector[0],
      mData[1] + vector[1],
      mData[2] + vector[2],
      mData[3] + vector[3]
    );
    return result;
  }

  Vec4<T> operator-(const Vec4<T>& vector) const {
    Vec4<T> result(
      mData[0] - vector[0],
      mData[1] - vector[1],
      mData[2] - vector[2],
      mData[3] - vector[3]
    );
    return result;
  }

  Vec4<T> operator*(T scalar) const {
    Vec4<T> result(
      mData[0] * scalar,
      mData[1] * scalar,
      mData[2] * scalar,
      mData[3] * scalar
    );
    return result;
  }

  T operator*(const Vec4<T>& vector) {
    T result = (mData[0] * vector[0]);
    result += (mData[1] * vector[1]);
    result += (mData[2] * vector[2]);
    result += (mData[3] * vector[3]);
    return result;
  }

  T operator*(const Vec4<T>& vector) const {
    T result = (mData[0] * vector[0]);
    result += (mData[1] * vector[1]);
    result += (mData[2] * vector[2]);
    result += (mData[3] * vector[3]);
    return result;
  }

  static T Length(const Vec4<T>& vector) {
    return NewtonRaphsonSqrt(vector * vector);
  }

  static void Normalize(Vec4<T>& vector) {
    T invSqrt(1 / Length(vector));
    vector[0] *= invSqrt;
    vector[1] *= invSqrt;
    vector[2] *= invSqrt;
    vector[3] *= invSqrt;
  }

  static void Homogenize(Vec4<T>& vector, std::size_t element) {
    T divisor(vector[element]);

    for (std::size_t i = 0; i <= element; i++) {
      vector[i] /= divisor;
    }
  }

  static void Clear(Vec4<T>& vector) {
    std::memset(*vector, 0, sizeof(T) * Elements);
  }

  private:
  static const std::size_t Elements = 4;
  T mData[Elements];
};

}
