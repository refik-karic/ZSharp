#include "Camera.h"

#include <cmath>

#include "Constants.h"
#include "Triangle.h"
#include "ZAlgorithm.h"
#include "ZConfig.h"

#define ASSERT_CHECK 0

namespace ZSharp {
Camera::Camera() {
  mLook[2] = 1.f;
  mUp[1] = 1.f;

  mFovHoriz = 90.f;
  mFovVert = 90.f;

  mNearPlane = 10.f;
  mFarPlane = 100.f;
}

Vec3 Camera::GetLook() const {
  return mLook;
}

Vec3 Camera::GetUp() const {
  return mUp;
}

void Camera::RotateCamera(const Mat4x4& rotationMatrix) {
  Vec4 rotatedVec(mLook);
  rotatedVec = rotationMatrix.ApplyTransform(rotatedVec);

  rotatedVec.Homogenize();
  mLook[0] = rotatedVec[0];
  mLook[1] = rotatedVec[1];
  mLook[2] = rotatedVec[2];

  rotatedVec = mUp;
  rotatedVec = rotationMatrix.ApplyTransform(rotatedVec);

  rotatedVec.Homogenize();
  mUp[0] = rotatedVec[0];
  mUp[1] = rotatedVec[1];
  mUp[2] = rotatedVec[2];
}

void Camera::PerspectiveProjection(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer) {
  Vec3 w(mLook);
  w.Normalize();

  Vec3 v(mUp - (w * (mUp * w)));
  v.Normalize();

  Vec3 u(v.Cross(w));

  Mat4x4 translation;
  translation.Identity();
  translation.SetTranslation(-mPosition);

  Mat4x4 uToE;
  uToE[0] = u;
  uToE[1] = v;
  uToE[2] = w;
  uToE[3][3] = 1.f;

  const float fovScale = (PI_OVER_180 / 2.f);
  Mat4x4 scale;
  scale[0][0] = 1.f / (mFarPlane * (tanf((mFovHoriz * fovScale))));
  scale[1][1] = 1.f / (mFarPlane * (tanf((mFovVert * fovScale))));
  scale[2][2] = 1.f / mFarPlane;
  scale[3][3] = 1.f;

  Mat4x4 unhing;
  unhing[0][0] = mFarPlane - mNearPlane;
  unhing[1][1] = mFarPlane - mNearPlane;
  unhing[2][2] = mFarPlane;
  unhing[2][3] = mNearPlane;
  unhing[3][2] = -(mFarPlane - mNearPlane);

  unhing = unhing * (scale * (uToE * translation));

  CullBackFacingPrimitives(vertexBuffer, indexBuffer, mPosition);

  for (size_t i = 0; i < vertexBuffer.GetVertSize(); ++i) {
    Vec4& vertexVector = *(reinterpret_cast<Vec4*>(vertexBuffer[i]));
    vertexVector = unhing.ApplyTransform(vertexVector);
    vertexVector.Homogenize();
  }

  ClipTriangles(vertexBuffer, indexBuffer);

  for (size_t i = 0; i < vertexBuffer.GetClipLength(); ++i) {
    float* vertexData = vertexBuffer.GetClipData(i);
    Vec3& vertexVector = *(reinterpret_cast<Vec3*>(vertexData));
    vertexVector.Homogenize();
    vertexVector = mWindowTransform.ApplyTransform(vertexVector);

#if ASSERT_CHECK
    const float width = (float)ZConfig::GetInstance().GetViewportWidth();
    const float height = (float)ZConfig::GetInstance().GetViewportHeight();

    // TODO: Window transform is not transforming point to correct screen space bounds.
    // Investigate if there's a bug here...
    ZAssert(vertexVector[0] >= 0.f);
    ZAssert(vertexVector[0] <= width);
    ZAssert(vertexVector[1] >= 0.f);
    ZAssert(vertexVector[1] <= height);
#endif
  }
}

void Camera::Resize() {
  float width = (float)ZConfig::GetInstance().GetViewportWidth();
  float height = (float)ZConfig::GetInstance().GetViewportHeight();

  mWindowTransform[0][0] = width;
  mWindowTransform[0][2] = width;
  mWindowTransform[1][1] = -height;
  mWindowTransform[1][2] = height;
  mWindowTransform = mWindowTransform * (1.f / 2.f);
}
  
}
