#include "Vec4.h"

#include <cmath>
#include <cstring>

#ifdef FORCE_SSE
#include "IntelIntrinsics.h"
#endif

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

Vec4::Vec4(const Vec4& copy) {
  *this = copy;
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
#ifdef FORCE_SSE
  Vec4 result;
  aligned_sse128addfloats(mData, vector.mData, result.mData);
  return result;
#else
  Vec4 result(
    mData[0] + vector[0],
    mData[1] + vector[1],
    mData[2] + vector[2],
    mData[3] + vector[3]
  );
  return result;
#endif
}

Vec4 Vec4::operator-(const Vec4& vector) const {
#ifdef FORCE_SSE
  Vec4 result;
  aligned_sse128subfloats(mData, vector.mData, result.mData);
  return result;
#else
  Vec4 result(
    mData[0] - vector[0],
    mData[1] - vector[1],
    mData[2] - vector[2],
    mData[3] - vector[3]
  );
  return result;
#endif
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
#ifdef FORCE_SSE
  Vec4 result;
  aligned_sse128mulfloat(mData, scalar, result.mData);
  return result;
#else
  Vec4 result(
    mData[0] * scalar,
    mData[1] * scalar,
    mData[2] * scalar,
    mData[3] * scalar
  );
  return result;
#endif
}

float Vec4::operator*(const Vec4& vector) const {
#ifdef FORCE_SSE
  return aligned_sse128mulfloatssum(mData, vector.mData);
#else
  float result = (mData[0] * vector[0]);
  result += (mData[1] * vector[1]);
  result += (mData[2] * vector[2]);
  result += (mData[3] * vector[3]);
  return result;
#endif
}

float Vec4::Length() const {
  return sqrtf((*this) * (*this));
}

void Vec4::Normalize() {
  const float invSqrt = 1.f / Length();
#ifdef FORCE_SSE
  aligned_sse128mulfloat(mData, invSqrt, mData);
#else
  mData[0] *= invSqrt;
  mData[1] *= invSqrt;
  mData[2] *= invSqrt;
  mData[3] *= invSqrt;
#endif
}

void Vec4::Homogenize() {
  const float invDivisor = 1.f / mData[3];
#ifdef FORCE_SSE
  // TODO: Figure out why the aligned version fails in release builds.
  // This class and the vertex buffer are both aligned to 16-bytes.
  unaligned_sse128mulfloat(mData, invDivisor, mData);
#else
  mData[0] *= invDivisor;
  mData[1] *= invDivisor;
  mData[2] *= invDivisor;
  mData[3] *= invDivisor;
#endif
}

void Vec4::Clear() {
  memset(mData, 0, sizeof(mData));
}

}
