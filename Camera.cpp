#include "Camera.h"

#include <cmath>

#include "Constants.h"
#include "Triangle.h"
#include "Win32PlatformApplication.h"
#include "ZAlgorithm.h"
#include "ZConfig.h"

#define ASSERT_CHECK 0

#define DISABLE_BACKFACE_CULLING 1

namespace ZSharp {
Camera::Camera() {
  mLook[2] = 1.f;
  mUp[1] = 1.f;

  mFovHoriz = 45.f;
  mFovVert = 45.f;

  mNearPlane = 0.02f;
  mFarPlane = 1000.f;

  const ZConfig& config = ZConfig::GetInstance();
  OnResize(config.GetViewportWidth(), config.GetViewportHeight());

  Win32PlatformApplication::OnWindowSizeChangedDelegate.Add(Delegate<size_t, size_t>::FromMember<Camera, &Camera::OnResize>(this));
}

Camera::~Camera() {
  Win32PlatformApplication::OnWindowSizeChangedDelegate.Remove(Delegate<size_t, size_t>::FromMember<Camera, &Camera::OnResize>(this));
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

#if 0
  rotatedVec = mUp;
  rotatedVec = rotationMatrix.ApplyTransform(rotatedVec);

  rotatedVec.Homogenize();
  mUp[0] = rotatedVec[0];
  mUp[1] = rotatedVec[1];
  mUp[2] = rotatedVec[2];
#endif
}

void Camera::PerspectiveProjection(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer) {
  Vec3 w(-mLook);
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

  const float standardNearPlane = -(mNearPlane / mFarPlane);
  const float standardFarPlane = -1.f;

  Mat4x4 unhing;
  unhing[0][0] = standardFarPlane - standardNearPlane;
  unhing[1][1] = standardFarPlane - standardNearPlane;
  unhing[2][2] = standardFarPlane;
  unhing[2][3] = standardNearPlane;
  unhing[3][2] = -(standardFarPlane - standardNearPlane);

  Mat4x4 perspectiveTransform(scale * (uToE * translation));
  perspectiveTransform = unhing * perspectiveTransform;

#if !DISABLE_BACKFACE_CULLING
  CullBackFacingPrimitives(vertexBuffer, indexBuffer, mPosition);
#endif

  for (size_t i = 0; i < vertexBuffer.GetVertSize(); ++i) {
    Vec4& vertexVector = *(reinterpret_cast<Vec4*>(vertexBuffer[i]));
    vertexVector = perspectiveTransform.ApplyTransform(vertexVector);
  }

  // Clip against near plane to avoid things behind camera reappearing.
  // This clip is special because it needs to append clip data and shuffle it back to the beginning.
  // The near clip edge must be > 0, hence we negate the plane.
  ClipTrianglesNearPlane(vertexBuffer, indexBuffer, -standardNearPlane);

  // Points at this stage must have Z > 0.
  // If Z < 0, points will re-appear in front of the camera.
  // If Z = 0, divide by 0 occurs and trashes the results.
  for (size_t i = 0; i < vertexBuffer.GetVertSize(); ++i) {
    Vec4& vertexVector = *(reinterpret_cast<Vec4*>(vertexBuffer[i]));
    vertexVector.Homogenize();
  }

  ClipTriangles(vertexBuffer, indexBuffer);

  for (size_t i = 0; i < vertexBuffer.GetClipLength(); ++i) {
    float* vertexData = vertexBuffer.GetClipData(i);
    Vec3& vertexVector = *(reinterpret_cast<Vec3*>(vertexData));

    const float perspectiveZ = vertexData[2];
    vertexVector.Homogenize();
    vertexVector = mWindowTransform.ApplyTransform(vertexVector);
    // Store perspective Z and inverse Z for each vertex in the clip buffer.
    // Prevents us from having to calculate this at a later point in the drawing code.
    vertexData[2] = perspectiveZ;
    vertexData[3] = (1 / perspectiveZ); // TODO: Is it necessary to negate this?

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

void Camera::OnResize(size_t width, size_t height) {
  mWindowTransform[0][0] = (float)width;
  mWindowTransform[0][2] = (float)width;
  mWindowTransform[1][1] = -((float)height);
  mWindowTransform[1][2] = (float)height;
  mWindowTransform = mWindowTransform * (1.f / 2.f);
}
  
}
