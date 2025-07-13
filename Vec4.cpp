#include "Vec4.h"

#include "Vec3.h"

#include <cmath>
#include <cstring>

#include "PlatformIntrinsics.h"

namespace ZSharp {

Vec4::Vec4() 
  : mData{} {
}

Vec4::Vec4(float* values)
  : mData{values[0], values[1], values[2], values[3]} {
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
  memcpy(mData, (const float*)(&vector), 3 * sizeof(float));
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

void Vec4::operator*=(float scalar) {
  Aligned_128MulByValueInPlace(mData, scalar);
}

float Vec4::operator*(const Vec4& vector) const {
  return Aligned_128MulSum(mData, vector.mData);
}

float Vec4::Length() const {
  return Unaligned_Vec4Length(mData);
}

void Vec4::Normalize() {
  Unaligned_Vec4Normalize(mData);
}

void Vec4::Homogenize() {
  Unaligned_Vec4Homogenize(mData);
}

void Vec4::Homogenize(Vec3& vec) const {
  Unaligned_Vec4HomogenizeToVec3(mData, *vec);
}

void Vec4::Clear() {
  memset(mData, 0, sizeof(mData));
}

String Vec4::ToString() const {
  return String::FromFormat("X={0}, Y={1}, Z={2}, W={3}", mData[0], mData[1], mData[2], mData[3]);
}

}
