﻿#pragma once

#include "ZBaseTypes.h"
#include "IndexBuffer.h"
#include "Mat2x3.h"
#include "Mat4x4.h"
#include "Quaternion.h"
#include "Vec3.h"
#include "VertexBuffer.h"
#include "WorldObject.h"

namespace ZSharp {

enum class ClipBounds {
  Inside,
  Outside,
  ClippedNear,
  ClippedNDC
};

class Camera final : public WorldObject {
  public:
  Camera();

  ~Camera();

  void ResetOrientation();

  Vec3 GetLook() const;

  Vec3 GetUp() const;

  void RotateCamera(const Quaternion& quat);

  void Tick();

  void PerspectiveProjection(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, ClipBounds clipBounds, const Mat4x4& objectTransform);

  ClipBounds ClipBoundsCheck(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, const Mat4x4& objectTransform);

  private:
  Vec3 mLook;
  Vec3 mUp;

  float mNearPlane;
  float mFarPlane;
  float mFovHoriz;
  float mFovVert;

  float mStandardNearPlane;

  float mWidth;
  float mHeight;

  Mat4x4 mPerspectiveTransform;
  Mat2x3 mWindowTransform;

  void OnResize(size_t width, size_t height);
};
}
