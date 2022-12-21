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

Vec3& WorldObject::Rotation() {
  return mRotation;
}

const Vec3& WorldObject::Rotation() const {
  return mRotation;
}

Mat4x4 WorldObject::ObjectTransform() const {
  Mat4x4 scale, rotationX, rotationY, rotationZ, resultTransform;
  scale.Identity();
  rotationX.Identity();
  rotationY.Identity();
  rotationZ.Identity();
  resultTransform.Identity();

  scale.SetScale(mScale);
  rotationX.SetRotation(mRotation[0], Mat4x4::Axis::X);
  rotationY.SetRotation(mRotation[1], Mat4x4::Axis::Y);
  rotationZ.SetRotation(mRotation[2], Mat4x4::Axis::Z);

  resultTransform = resultTransform * scale;
  resultTransform = resultTransform * rotationX;
  resultTransform = resultTransform * rotationY;
  resultTransform = resultTransform * rotationZ;
  return resultTransform;
}

}
