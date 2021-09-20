#pragma once

#include "Mat4x4.h"
#include "Vec3.h"
#include "Vec4.h"

namespace ZSharp {
class Quaternion {
  public:
  enum QuatAxis {
    W,
    X,
    Y,
    Z
  };

  Quaternion(float theta, const Vec3& axes);

  Mat4x4 GetRotationMatrix() const;

  private:
  Vec4 mAngles;
};
}
