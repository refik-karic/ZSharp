#include "Mat4x4.h"

#include <cmath>
#include <cstring>

#include "PlatformIntrinsics.h"

namespace ZSharp {

Mat4x4::Mat4x4() {
}

Mat4x4::Mat4x4(const Vec4& a, const Vec4& b, const Vec4& c, const Vec4& d) 
  : mData{a, b, c, d} {
}

Mat4x4::Mat4x4(const Mat4x4& copy)
  : mData{copy.mData[0], copy.mData[1], copy.mData[2], copy.mData[3]} {
}

void Mat4x4::operator=(const Mat4x4& matrix) {
  if (this == &matrix) {
    return;
  }

  memcpy(mData, matrix.mData, sizeof(mData));
}

Vec4& Mat4x4::operator[](size_t index) {
  return mData[index];
}

const Vec4& Mat4x4::operator[](size_t index) const {
  return mData[index];
}

float* Mat4x4::operator*() {
  return (float*)&mData[0];
}

const float* Mat4x4::operator*() const {
  return (float*)&mData[0];
}

Mat4x4 Mat4x4::operator*(float scalar) {
  Mat4x4 result;

  result[0] = mData[0] * scalar;
  result[1] = mData[1] * scalar;
  result[2] = mData[2] * scalar;
  result[3] = mData[3] * scalar;

  return result;
}

Mat4x4 Mat4x4::operator*(const Mat4x4& matrix) const {
  Mat4x4 result;
  Unaligned_Mat4x4Mul((const float*)mData, (const float*)matrix.mData, (float*)result.mData);
  return result;
}

Mat4x4& Mat4x4::operator*=(const Mat4x4& matrix) {
  Unaligned_Mat4x4MulInPlace((float*)mData, (const float*)matrix.mData);
  return *this;
}

void Mat4x4::Identity() {
  Clear();
  mData[0][0] = 1.f;
  mData[1][1] = 1.f;
  mData[2][2] = 1.f;
  mData[3][3] = 1.f;
}

void Mat4x4::Clear() {
  memset(mData, 0, sizeof(mData));
}

Mat4x4 Mat4x4::Transpose() const {
  Mat4x4 result;
  
  result[0][0] = mData[0][0];
  result[0][1] = mData[1][0];
  result[0][2] = mData[2][0];
  result[0][3] = mData[3][0];

  result[1][0] = mData[0][1];
  result[1][1] = mData[1][1];
  result[1][2] = mData[2][1];
  result[1][3] = mData[3][1];

  result[2][0] = mData[0][2];
  result[2][1] = mData[1][2];
  result[2][2] = mData[2][2];
  result[2][3] = mData[3][2];

  result[3][0] = mData[0][3];
  result[3][1] = mData[1][3];
  result[3][2] = mData[2][3];
  result[3][3] = mData[3][3];

  return result;
}

void Mat4x4::SetTranslation(const Vec3& translation) {
  mData[0][3] = translation[0];
  mData[1][3] = translation[1];
  mData[2][3] = translation[2];
}

void Mat4x4::SetTranslation(const Vec4& translation) {
  mData[0][3] = translation[0];
  mData[1][3] = translation[1];
  mData[2][3] = translation[2];
  mData[3][3] = translation[3];
}

void Mat4x4::SetScale(const Vec4& scale) {
  mData[0][0] = scale[0];
  mData[1][1] = scale[1];
  mData[2][2] = scale[2];
  mData[3][3] = scale[3];
}

void Mat4x4::SetRotation(float angle, Axis axis) {
  switch (axis) {
  case Axis::Z:
    mData[0][0] = cosf(angle);
    mData[0][1] = -sinf(angle);
    mData[1][0] = sinf(angle);
    mData[1][1] = cosf(angle);
    mData[2][2] = 1.f;
    mData[3][3] = 1.f;
    break;
  case Axis::X:
    mData[0][0] = 1.f;
    mData[1][1] = cosf(angle);
    mData[1][2] = -sinf(angle);
    mData[2][1] = sinf(angle);
    mData[2][2] = cosf(angle);
    mData[3][3] = 1.f;
    break;
  case Axis::Y:
    mData[0][0] = cosf(angle);
    mData[0][2] = sinf(angle);
    mData[1][1] = 1.f;
    mData[2][0] = -sinf(angle);
    mData[2][2] = cosf(angle);
    mData[3][3] = 1.f;
    break;
  }
}

Vec4 Mat4x4::ApplyTransform(const Vec4& domain) const {
  Vec4 result;
  Unaligned_Mat4x4Vec4Transform((const float(*)[4])mData, *domain, *result);
  return result;
}

}
