#include "Vec3.h"

#include "CommonMath.h"
#include "PlatformIntrinsics.h"

#include <cmath>
#include <cstring>

namespace ZSharp {

Vec3::Vec3() {
  Clear();
}

Vec3::Vec3(float* values) 
  : mData{values[0], values[1], values[2]} {
}

Vec3::Vec3(float value) 
  : mData{value, value, value} {
}

Vec3::Vec3(float x, float y, float z)
  : mData{x, y, z} {
}

Vec3::Vec3(const Vec3& copy)
  : mData{copy.mData[0], copy.mData[1], copy.mData[2]} {
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
  return mData;
}

const float* Vec3::operator*() const {
  return mData;
}

float Vec3::operator[](const size_t index) const {
  return mData[index];
}

float& Vec3::operator[](const size_t index) {
  return mData[index];
}

Vec3 Vec3::operator+(const Vec3& vector) const {
  Vec3 result(
    mData[0] + vector.mData[0],
    mData[1] + vector.mData[1],
    mData[2] + vector.mData[2]
  );
  return result;
}

void Vec3::operator+=(const Vec3& vector) {
  mData[0] += vector.mData[0];
  mData[1] += vector.mData[1];
  mData[2] += vector.mData[2];
}

void Vec3::operator*=(float scalar) {
  mData[0] *= scalar;
  mData[1] *= scalar;
  mData[2] *= scalar;
}

Vec3 Vec3::operator-(const Vec3& vector) const {
  Vec3 result(
    mData[0] - vector.mData[0],
    mData[1] - vector.mData[1],
    mData[2] - vector.mData[2]
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
  return Dot3(mData, vector.mData);
}

Vec3 Vec3::Cross(const Vec3& vec) const {
  Vec3 result(
    (mData[1] * vec.mData[2]) - (mData[2] * vec.mData[1]),
    (mData[2] * vec.mData[0]) - (mData[0] * vec.mData[2]),
    (mData[0] * vec.mData[1]) - (mData[1] * vec.mData[0])
  );
  return result;
}

Vec3 Vec3::TripleCross(const Vec3& a, const Vec3& b) const {
  // TODO: Rewrite this using intrinsics.
  const float xyz[3] = {
    (mData[1] * a.mData[2]) - (mData[2] * a.mData[1]),
    (mData[2] * a.mData[0]) - (mData[0] * a.mData[2]),
    (mData[0] * a.mData[1]) - (mData[1] * a.mData[0])
  };

  Vec3 result(
    (xyz[1] * b.mData[2]) - (xyz[2] * b.mData[1]),
    (xyz[2] * b.mData[0]) - (xyz[0] * b.mData[2]),
    (xyz[0] * b.mData[1]) - (xyz[1] * b.mData[0])
  );

  return result;
}

Vec3 Vec3::Parametric(const Vec3& rhs, float t) const {
  Vec3 result((rhs.mData[0] * t) + mData[0],
    (rhs.mData[1] * t) + mData[1],
    (rhs.mData[2] * t) + mData[2]);
  
  return result;
}

float Vec3::Length() const {
  return Unaligned_Vec3Length(mData);
}

void Vec3::Normalize() {
  Unaligned_Vec3Normalize(mData);
}

void Vec3::Homogenize() {
  const float divisor = mData[2];
  mData[0] /= divisor;
  mData[1] /= divisor;
  mData[2] /= divisor;
}

void Vec3::Clear() {
  memset(mData, 0, sizeof(mData));
}

String Vec3::ToString() const {
  return String::FromFormat("X={0:4}, Y={1:4}, Z={2:4}", mData[0], mData[1], mData[2]);
}

}
