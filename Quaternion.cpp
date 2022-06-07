#include "Quaternion.h"

#include <cmath>

namespace ZSharp {
Quaternion::Quaternion() {

}

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

Quaternion Quaternion::operator-(const Quaternion& rhs) const {
  Quaternion result(mAngles - rhs.mAngles);
  return result;
}

Quaternion Quaternion::operator*(const Quaternion& rhs) const {
  const float W = (mAngles[QuatAxis::W] * rhs.mAngles[QuatAxis::W])
    - (mAngles[QuatAxis::X] * rhs.mAngles[QuatAxis::X])
    - (mAngles[QuatAxis::Y] * rhs.mAngles[QuatAxis::Y])
    - (mAngles[QuatAxis::Z] * rhs.mAngles[QuatAxis::Z]);
  
  const float X = (mAngles[QuatAxis::W] * rhs.mAngles[QuatAxis::X])
    + (mAngles[QuatAxis::X] * rhs.mAngles[QuatAxis::W])
    + (mAngles[QuatAxis::Y] * rhs.mAngles[QuatAxis::Z])
    - (mAngles[QuatAxis::Z] * rhs.mAngles[QuatAxis::Y]);

  const float Y = (mAngles[QuatAxis::W] * rhs.mAngles[QuatAxis::Y])
    - (mAngles[QuatAxis::X] * rhs.mAngles[QuatAxis::Z])
    + (mAngles[QuatAxis::Y] * rhs.mAngles[QuatAxis::W])
    + (mAngles[QuatAxis::Z] * rhs.mAngles[QuatAxis::X]);

  const float Z = (mAngles[QuatAxis::W] * rhs.mAngles[QuatAxis::Z])
    + (mAngles[QuatAxis::X] * rhs.mAngles[QuatAxis::Y])
    - (mAngles[QuatAxis::Y] * rhs.mAngles[QuatAxis::X])
    + (mAngles[QuatAxis::Z] * rhs.mAngles[QuatAxis::W]);

  Vec4 multiplyResult(W, X, Y, Z);
  Quaternion result(multiplyResult);
  return result;
}

Mat4x4 Quaternion::GetRotationMatrix() const {
  Mat4x4 result;
  result[0][0] = 1.f - (2.f * (mAngles[QuatAxis::Y] * mAngles[QuatAxis::Y])) - (2.f * (mAngles[QuatAxis::Z] * mAngles[QuatAxis::Z]));
  result[0][1] = (2.f * mAngles[QuatAxis::X] * mAngles[QuatAxis::Y]) - (2.f * mAngles[QuatAxis::W] * mAngles[QuatAxis::Z]);
  result[0][2] = (2.f * mAngles[QuatAxis::X] * mAngles[QuatAxis::Z]) + (2.f * mAngles[QuatAxis::W] * mAngles[QuatAxis::Y]);
  result[1][0] = (2.f * mAngles[QuatAxis::X] * mAngles[QuatAxis::Y]) + (2.f * mAngles[QuatAxis::W] * mAngles[QuatAxis::Z]);
  result[1][1] = 1.f - (2.f * (mAngles[QuatAxis::X] * mAngles[QuatAxis::X])) - (2.f * (mAngles[QuatAxis::Z] * mAngles[QuatAxis::Z]));
  result[1][2] = (2.f * mAngles[QuatAxis::Y] * mAngles[QuatAxis::Z]) + (2.f * mAngles[QuatAxis::W] * mAngles[QuatAxis::X]);
  result[2][0] = (2.f * mAngles[QuatAxis::X] * mAngles[QuatAxis::Z]) - (2.f * mAngles[QuatAxis::W] * mAngles[QuatAxis::Y]);
  result[2][1] = (2.f * mAngles[QuatAxis::Y] * mAngles[QuatAxis::Z]) - (2.f * mAngles[QuatAxis::W] * mAngles[QuatAxis::X]);
  result[2][2] = 1.f - (2.f * (mAngles[QuatAxis::X] * mAngles[QuatAxis::X])) - (2.f * (mAngles[QuatAxis::Y] * mAngles[QuatAxis::Y]));
  result[3][3] = 1.f;
  return result;
}

const Vec4& Quaternion::Vector() {
  return mAngles;
}
}
