#pragma once

#include "ZBaseTypes.h"
#include "Mat4x4.h"
#include "Vec3.h"

namespace ZSharp {

class WorldObject {
  public:

  WorldObject();

  Vec3& Position();

  const Vec3& Position() const;

  Vec3& Scale();

  const Vec3& Scale() const;

  Vec3& Rotation();

  const Vec3& Rotation() const;

  Vec3& Translation();

  const Vec3& Translation() const;

  Mat4x4 ObjectTransform() const;

  protected:
  Vec3 mPosition;
  Vec3 mScale;
  Vec3 mRotation;
  Vec3 mTranslation;
};

}
