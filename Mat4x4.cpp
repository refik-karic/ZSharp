#include "Mat4x4.h"

#include <cmath>

namespace ZSharp {

Mat4x4::Mat4x4() {
}

Mat4x4::Mat4x4(const Mat4x4& copy) {
  *this = copy;
}

void Mat4x4::Identity() {
  Clear();
  mData[0][0] = 1.f;
  mData[1][1] = 1.f;
  mData[2][2] = 1.f;
  mData[3][3] = 1.f;
}

void Mat4x4::Clear() {
  mData[0].Clear();
  mData[1].Clear();
  mData[2].Clear();
  mData[3].Clear();
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
  Vec4 codomainResult;
  codomainResult[0] = domain * mData[0];
  codomainResult[1] = domain * mData[1];
  codomainResult[2] = domain * mData[2];
  codomainResult[3] = domain * mData[3];
  return codomainResult;
}

}
