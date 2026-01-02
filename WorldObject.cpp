#include "WorldObject.h"

namespace ZSharp {

WorldObject::WorldObject()
  : mScale(1.f) {

}

Vec3& WorldObject::Position() {
  return mPosition;
}

const Vec3& WorldObject::Position() const {
  return mPosition;
}

Vec3& WorldObject::Scale() {
  return mScale;
}

const Vec3& WorldObject::Scale() const {
  return mScale;
}

Quaternion& WorldObject::Rotation() {
  return mRotation;
}

const Quaternion& WorldObject::Rotation() const {
  return mRotation;
}

Vec3& WorldObject::Translation() {
  return mTranslation;
}

const Vec3& WorldObject::Translation() const {
  return mTranslation;
}

Mat4x4 WorldObject::ObjectTransform() const {
  Mat4x4 srt(mScale, mRotation, mTranslation + mPosition);
  return srt;
}

}
