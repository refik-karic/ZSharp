#include "Quaternion.h"

#include "Mat4x4.h"
#include "PlatformIntrinsics.h"

#include <cmath>

namespace ZSharp {
Quaternion::Quaternion() {}

Quaternion::Quaternion(const Quaternion& rhs) {
  mAngles = rhs.mAngles;
}

void Quaternion::operator=(const Quaternion& rhs) {
  if (this != &rhs) {
    mAngles = rhs.mAngles;
  }
}

Quaternion::Quaternion(float theta, const Vec3& axes) {
  mAngles[QuatAxis::W] = cosf(theta / 2.f);
  mAngles[QuatAxis::X] = sinf(theta / 2.f) * axes[0];
  mAngles[QuatAxis::Y] = sinf(theta / 2.f) * axes[1];
  mAngles[QuatAxis::Z] = sinf(theta / 2.f) * axes[2];
}

Quaternion::Quaternion(const Vec4& axes) {
  mAngles = axes;
}

float Quaternion::operator[](size_t index) const {
  return mAngles[index];
}

Quaternion Quaternion::operator+(const Quaternion& rhs) const {
  Quaternion result(mAngles + rhs.mAngles);
  return result;
}

Quaternion& Quaternion::operator+=(const Quaternion& rhs) {
  mAngles += rhs.mAngles;
  return *this;
}

Quaternion Quaternion::operator-(const Quaternion& rhs) const {
  Quaternion result(mAngles - rhs.mAngles);
  return result;
}

Quaternion Quaternion::operator*(const Quaternion& rhs) const {
  Quaternion result;
  float* resultData = (float*)(&(result.mAngles));
  resultData[QuatAxis::W] = (mAngles[QuatAxis::W] * rhs.mAngles[QuatAxis::W])
    - (mAngles[QuatAxis::X] * rhs.mAngles[QuatAxis::X])
    - (mAngles[QuatAxis::Y] * rhs.mAngles[QuatAxis::Y])
    - (mAngles[QuatAxis::Z] * rhs.mAngles[QuatAxis::Z]);
  
  resultData[QuatAxis::X] = (mAngles[QuatAxis::W] * rhs.mAngles[QuatAxis::X])
    + (mAngles[QuatAxis::X] * rhs.mAngles[QuatAxis::W])
    + (mAngles[QuatAxis::Y] * rhs.mAngles[QuatAxis::Z])
    - (mAngles[QuatAxis::Z] * rhs.mAngles[QuatAxis::Y]);

  resultData[QuatAxis::Y] = (mAngles[QuatAxis::W] * rhs.mAngles[QuatAxis::Y])
    - (mAngles[QuatAxis::X] * rhs.mAngles[QuatAxis::Z])
    + (mAngles[QuatAxis::Y] * rhs.mAngles[QuatAxis::W])
    + (mAngles[QuatAxis::Z] * rhs.mAngles[QuatAxis::X]);

  resultData[QuatAxis::Z] = (mAngles[QuatAxis::W] * rhs.mAngles[QuatAxis::Z])
    + (mAngles[QuatAxis::X] * rhs.mAngles[QuatAxis::Y])
    - (mAngles[QuatAxis::Y] * rhs.mAngles[QuatAxis::X])
    + (mAngles[QuatAxis::Z] * rhs.mAngles[QuatAxis::W]);

  return result;
}

Mat4x4 Quaternion::GetRotationMatrix() const {
  Mat4x4 result;
  Unaligned_QuaternionToMat4x4((const float*)&mAngles, (float(*)[4])&result);
  return result;
}

Mat4x4 Quaternion::GetScaledRotationMatrix(const Vec3& scale) const {
  Mat4x4 result;
  Unaligned_QuaternionToMat4x4_Scaled((const float*)&mAngles, (float(*)[4])&result, (const float*)&scale);
  return result;
}

void Quaternion::GetScaledRotationMatrix(const Vec3& scale, Mat4x4& mat) const {
  Unaligned_QuaternionToMat4x4_Scaled((const float*)&mAngles, (float(*)[4])&mat, (const float*)&scale);
}

const Vec4& Quaternion::Vector() {
  return mAngles;
}
}
