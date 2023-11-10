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

Vec3& WorldObject::Translation() {
  return mTranslation;
}

const Vec3& WorldObject::Translation() const {
  return mTranslation;
}

Mat4x4 WorldObject::ObjectTransform() const {
  Mat4x4 position, scale, rotationX, rotationY, rotationZ, translation, resultTransform;
  position.Identity();
  scale.Identity();
  rotationX.Identity();
  rotationY.Identity();
  rotationZ.Identity();
  resultTransform.Identity();
  translation.Identity();

  position.SetTranslation(mPosition);
  scale.SetScale(mScale);
  rotationX.SetRotation(mRotation[0], Mat4x4::Axis::X);
  rotationY.SetRotation(mRotation[1], Mat4x4::Axis::Y);
  rotationZ.SetRotation(mRotation[2], Mat4x4::Axis::Z);

  translation.SetTranslation(mTranslation);

  resultTransform = resultTransform * position;
  resultTransform = resultTransform * scale;
  resultTransform = resultTransform * rotationX;
  resultTransform = resultTransform * rotationY;
  resultTransform = resultTransform * rotationZ;
  resultTransform = resultTransform * translation;
  return resultTransform;
}

}
