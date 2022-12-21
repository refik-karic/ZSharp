#include "Vec3.h"

#include <cmath>
#include <cstring>

namespace ZSharp {

Vec3::Vec3() {
  Clear();
}

Vec3::Vec3(float value) 
  : mData{value, value, value} {
}

Vec3::Vec3(float x, float y, float z)
  : mData{x, y, z} {
}

Vec3::Vec3(const Vec3& copy) {
  *this = copy;
}

void Vec3::operator=(const Vec3& vector) {
  if (this == &vector) {
    return;
  }

  memcpy(mData, *vector, sizeof(mData));
}

bool Vec3::operator==(const Vec3& vector) const {
  if (this == &vector) {
    return true;
  }

  return memcmp(mData, *vector, sizeof(mData)) == 0;
}

float* Vec3::operator*() {
  return &mData[0];
}

const float* Vec3::operator*() const {
  return &mData[0];
}

float Vec3::operator[](const size_t index) const {
  return mData[index];
}

float& Vec3::operator[](const size_t index) {
  return mData[index];
}

Vec3 Vec3::operator+(const Vec3& vector) const {
  Vec3 result(
    mData[0] + vector[0],
    mData[1] + vector[1],
    mData[2] + vector[2]
  );
  return result;
}

Vec3 Vec3::operator-(const Vec3& vector) const {
  Vec3 result(
    mData[0] - vector[0],
    mData[1] - vector[1],
    mData[2] - vector[2]
  );
  return result;
}

Vec3 Vec3::operator-() const {
  Vec3 result(
    -mData[0],
    -mData[1],
    -mData[2]
  );
  return result;
}

Vec3 Vec3::operator*(float scalar) const {
  Vec3 result(mData[0] * scalar, mData[1] * scalar, mData[2] * scalar);
  return result;
}

float Vec3::operator*(const Vec3& vector) const {
  return (mData[0] * vector[0]) + (mData[1] * vector[1]) + (mData[2] * vector[2]);
}

Vec3 Vec3::Cross(const Vec3& vec) const {
  Vec3 result(
    (mData[1] * vec[2]) - (mData[2] * vec[1]),
    (mData[2] * vec[0]) - (mData[0] * vec[2]),
    (mData[0] * vec[1]) - (mData[1] * vec[0])
  );
  return result;
}

float Vec3::Length() const {
  return sqrtf((*this) * (*this));
}

void Vec3::Normalize() {
  const float invSqrt(1.f / Length());
  mData[0] *= invSqrt;
  mData[1] *= invSqrt;
  mData[2] *= invSqrt;
}

void Vec3::Homogenize() {
  const float invDivisor = 1.f / mData[2];
  mData[0] *= invDivisor;
  mData[1] *= invDivisor;
  mData[2] *= invDivisor;
}

void Vec3::Clear() {
  memset(mData, 0, sizeof(mData));
}

}
