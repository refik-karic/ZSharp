#pragma once

#include <cstdint>

#include "IndexBuffer.h"
#include "Mat4x4.h"
#include "Vec3.h"
#include "VertexBuffer.h"

namespace ZSharp {

class Camera final {
  public:
  Camera();

  Vec3 GetPosition() const;

  void MoveCamera(const Vec3& position);

  void RotateCamera(const Mat4x4& rotationMatrix);

  void PerspectiveProjection(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer);

  private:
  Vec3 mPosition;
  Vec3 mLook;
  Vec3 mUp;

  float mNearPlane;
  float mFarPlane;
  float mFovHoriz;
  float mFovVert;

  size_t mWidth;
  size_t mHeight;
};
}
