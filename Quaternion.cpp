#include "Quaternion.h"

#include <cmath>

namespace ZSharp {
Quaternion::Quaternion(float theta, const Vec3& axes) {
  mAngles[QuatAxis::W] = cosf(theta / 2.f);
  mAngles[QuatAxis::X] = sinf(theta / 2.f) * axes[0];
  mAngles[QuatAxis::Y] = sinf(theta / 2.f) * axes[1];
  mAngles[QuatAxis::Z] = sinf(theta / 2.f) * axes[2];
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
