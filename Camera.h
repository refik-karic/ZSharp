#pragma once

#include "ZBaseTypes.h"
#include "IndexBuffer.h"
#include "Mat2x3.h"
#include "Mat4x4.h"
#include "Vec3.h"
#include "VertexBuffer.h"
#include "WorldObject.h"

namespace ZSharp {

class Camera final : public WorldObject {
  public:
  Camera();

  ~Camera();

  Vec3 GetLook() const;

  Vec3 GetUp() const;

  void RotateCamera(const Mat4x4& rotationMatrix);

  void PerspectiveProjection(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer);

  private:
  Vec3 mLook;
  Vec3 mUp;

  float mNearPlane;
  float mFarPlane;
  float mFovHoriz;
  float mFovVert;

  Mat2x3 mWindowTransform;

  void OnResize(size_t width, size_t height);
};
}
