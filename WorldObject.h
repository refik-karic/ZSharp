#pragma once

#include "ZBaseTypes.h"
#include "Vec3.h"

namespace ZSharp {

class WorldObject {
  public:

  Vec3& Position() {
    return mPosition;
  }

  const Vec3& Position() const {
    return mPosition;
  }

  protected:
  Vec3 mPosition;
};

}
