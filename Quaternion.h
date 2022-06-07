#pragma once

#include "Mat4x4.h"
#include "Vec3.h"
#include "Vec4.h"
#include "ZBaseTypes.h"

namespace ZSharp {

// Note that angles are interpreted and stored internally as radians.
class Quaternion final {
  public:
  enum QuatAxis {
    W,
    X,
    Y,
    Z
  };

  Quaternion();

  Quaternion(const Quaternion& rhs);

  void operator=(const Quaternion& rhs);

  // Axis angle.
  Quaternion(float theta, const Vec3& axes);

  // W, I, J, K initialization.
  Quaternion(const Vec4& axes);

  float operator[](size_t index) const;

  Quaternion operator+(const Quaternion& rhs) const;

  Quaternion operator-(const Quaternion& rhs) const;

  Quaternion operator*(const Quaternion& rhs) const;

  Mat4x4 GetRotationMatrix() const;

  const Vec4& Vector();

  private:
  Vec4 mAngles;
};
}
