#include "Vec4.h"

#include <cmath>
#include <cstring>

#include "PlatformIntrinsics.h"

namespace ZSharp {

Vec4::Vec4() {
  Clear();
}

Vec4::Vec4(float value)
  : mData{value, value, value, value} {
}

Vec4::Vec4(float x, float y, float z, float w)
  : mData{x, y, z, w} {
}

Vec4::Vec4(const Vec4& copy)
  : mData{copy.mData[0], copy.mData[1], copy.mData[2], copy.mData[3]} {
}

Vec4::Vec4(const Vec3& copy) {
  *this = copy;
}

Vec4::Vec4(const Vec3& copy, float w) 
  : mData{copy[0], copy[1], copy[2], w} {
}

void Vec4::operator=(const Vec3& vector) {
  memcpy(mData, *vector, sizeof(mData));
  mData[3] = 1.f;
}

void Vec4::operator=(const Vec4& vector) {
  if (this == &vector) {
    return;
  }

  memcpy(mData, *vector, sizeof(mData));
}

bool Vec4::operator==(const Vec4& vector) const {
  if (this == &vector) {
    return true;
  }

  return memcmp(mData, *vector, sizeof(mData)) == 0;
}

float* Vec4::operator*() {
  return &mData[0];
}

const float* Vec4::operator*() const {
  return &mData[0];
}

float Vec4::operator[](const size_t index) const {
  return mData[index];
}

float& Vec4::operator[](const size_t index) {
  return mData[index];
}

Vec4 Vec4::operator+(const Vec4& vector) const {
  Vec4 result;
  Aligned_128Add(mData, vector.mData, result.mData);
  return result;
}

Vec4 Vec4::operator-(const Vec4& vector) const {
  Vec4 result;
  // TODO: Figure out why this crashes even though it's aligned.
  //Aligned_128Sub(mData, vector.mData, result.mData);
  Unaligned_128Sub(mData, vector.mData, result.mData);
  return result;
}

Vec4 Vec4::operator-() const {
  Vec4 result(
    -mData[0],
    -mData[1],
    -mData[2],
    -mData[3]
  );
  return result;
}

Vec4 Vec4::operator*(float scalar) const {
  Vec4 result;
  Aligned_128MulByValue(mData, scalar, result.mData);
  return result;
}

float Vec4::operator*(const Vec4& vector) const {
  return Aligned_128MulSum(mData, vector.mData);
}

float Vec4::Length() const {
  return sqrtf((*this) * (*this));
}

void Vec4::Normalize() {
  const float invSqrt = 1.f / Length();
  Aligned_128MulByValue(mData, invSqrt, mData);
}

void Vec4::Homogenize() {
  const float invDivisor = 1.f / mData[3];
  // TODO: Figure out why the aligned version fails in release builds.
  // This class and the vertex buffer are both aligned to 16-bytes.
  Unaligned_128MulByValue(mData, invDivisor, mData);
}

void Vec4::Clear() {
  memset(mData, 0, sizeof(mData));
}

String Vec4::ToString() const {
  String result(String::FromFormat("X={0}, Y={1}, Z={2}, W={3}", mData[0], mData[1], mData[2], mData[3]));
  return result;
}

}
