#pragma once

#include "ZBaseTypes.h"
#include "Mat4x4.h"
#include "Quaternion.h"
#include "Vec3.h"

namespace ZSharp {

class WorldObject {
  public:

  WorldObject();

  Vec3& Position();

  const Vec3& Position() const;

  Vec3& Scale();

  const Vec3& Scale() const;

  Quaternion& Rotation();

  const Quaternion& Rotation() const;

  Vec3& Translation();

  const Vec3& Translation() const;

  Mat4x4 ObjectTransform() const;

  protected:
  Vec3 mPosition;
  Vec3 mScale;
  Quaternion mRotation;
  Vec3 mTranslation;
};

}
