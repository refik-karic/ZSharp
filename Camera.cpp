#include "Camera.h"

#include <cmath>

#include "CommonMath.h"
#include "Constants.h"
#include "ConsoleVariable.h"
#include "PlatformApplication.h"
#include "ZAlgorithm.h"
#include "ZConfig.h"
#include "PlatformIntrinsics.h"
#include "ScopedTimer.h"

#define ROTATE_CAMERA_WORLD_CENTER 1

namespace ZSharp {
ConsoleVariable<bool> BackfaceCull("BackfaceCull", true);

Camera::Camera() {
  mLook[2] = -1.f;
  mUp[1] = 1.f;

  mFovHoriz = 70.f;
  mFovVert = 45.f;

  //mNearPlane = 0.02f;
  //mFarPlane = 1000.f;

  mNearPlane = 10.f;
  mFarPlane = 1000.f;

  const ZConfig& config = ZConfig::Get();
  OnResize(config.GetViewportWidth().Value(), config.GetViewportHeight().Value());

  OnWindowSizeChangedDelegate().Add(Delegate<size_t, size_t>::FromMember<Camera, &Camera::OnResize>(this));
}

Camera::~Camera() {
  OnWindowSizeChangedDelegate().Remove(Delegate<size_t, size_t>::FromMember<Camera, &Camera::OnResize>(this));
}

void Camera::ResetOrientation() {
  mLook.Clear();
  mUp.Clear();
  mLook[2] = -1.f;
  mUp[1] = 1.f;
}

Vec3 Camera::GetLook() const {
  return mLook;
}

Vec3 Camera::GetUp() const {
  return mUp;
}

void Camera::RotateCamera(const Mat4x4& rotationMatrix) {
#if 1
  Vec4 lookVec(mLook);
  lookVec = rotationMatrix.ApplyTransform(lookVec);

  lookVec.Homogenize();
  mLook[0] = lookVec[0];
  mLook[1] = lookVec[1];
  mLook[2] = lookVec[2];
#endif

#if ROTATE_CAMERA_WORLD_CENTER
  Vec4 positionVec(mPosition);
  positionVec = rotationMatrix.ApplyTransform(positionVec);

  positionVec.Homogenize();
  mPosition[0] = positionVec[0];
  mPosition[1] = positionVec[1];
  mPosition[2] = positionVec[2];
#endif

#if 0
  Vec4 upVec(mUp);
  upVec = rotationMatrix.ApplyTransform(upVec);

  upVec.Homogenize();
  mUp[0] = upVec[0];
  mUp[1] = upVec[1];
  mUp[2] = upVec[2];
#endif
}

void Camera::Tick() {
  NamedScopedTimer(CameraTick);

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

  mStandardNearPlane = -(mNearPlane / mFarPlane);
  const float standardFarPlane = -1.f;
  const float standardNearPlane = -.001f;

  Mat4x4 unhing;
  unhing[0][0] = standardFarPlane;
  unhing[1][1] = standardFarPlane;
  unhing[2][2] = standardFarPlane;
  unhing[2][3] = standardNearPlane;
  unhing[3][2] = -(standardFarPlane - standardNearPlane);

  mPerspectiveTransform = (unhing * (scale * (uToE * translation)));
}

void Camera::PerspectiveProjection(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, ClipBounds clipBounds, const Mat4x4& objectTransform) {
  NamedScopedTimer(PerspectiveProjection);

  if (clipBounds == ClipBounds::Outside) {
    vertexBuffer.Reset();
    indexBuffer.Reset();
    return;
  }

  const int32 stride = vertexBuffer.GetStride();
  const float* windowTransformVec0 = *mWindowTransform[0];
  const float* windowTransformVec1 = *mWindowTransform[1];

  if (clipBounds == ClipBounds::Inside) {
    NamedScopedTimer(TransformDirect);

    Aligned_Mat4x4Transform((const float(*)[4])*(mPerspectiveTransform * objectTransform), vertexBuffer[0], stride, vertexBuffer.GetVertSize() * stride);

    if (*BackfaceCull) {
      CullBackFacingPrimitives(vertexBuffer, indexBuffer);
    }

    Aligned_HomogenizeTransformScreenSpace(vertexBuffer[0], stride, vertexBuffer.GetVertSize() * stride, windowTransformVec0, windowTransformVec1, mWidth, mHeight);
  }
  else {
    // Apply the perspective projection transform to all input vertices.
    Aligned_Mat4x4Transform((const float(*)[4])*(mPerspectiveTransform * objectTransform), vertexBuffer[0], stride, vertexBuffer.GetVertSize() * stride);

    if (*BackfaceCull) {
      CullBackFacingPrimitives(vertexBuffer, indexBuffer);
    }

    // Clip against near plane to avoid things behind camera reappearing.
    // This clip is special because it needs to append clip data and shuffle it back to the beginning.
    // The near clip edge must be > 0, hence we negate the plane.
    if (clipBounds == ClipBounds::ClippedNear) {
      ClipTrianglesNearPlane(vertexBuffer, indexBuffer, -mStandardNearPlane);
    }

    // Points at this stage must have Z > 0.
    // If Z < 0, points will re-appear in front of the camera.
    // If Z = 0, divide by 0 occurs and trashes the results.
    Aligned_Vec4Homogenize(vertexBuffer[0], stride, vertexBuffer.GetVertSize() * stride);

    if (clipBounds != ClipBounds::Inside) {
      ClipTriangles(vertexBuffer, indexBuffer);
      vertexBuffer.WasClipped() = true;
      indexBuffer.WasClipped() = true;
    }

    int32 vertClipLength = 0;
    float* vertexClipData = nullptr;

    if (vertexBuffer.WasClipped()) {
      vertClipLength = vertexBuffer.GetClipLength();
      vertexClipData = vertexBuffer.GetClipData(0);
    }
    else {
      vertClipLength = vertexBuffer.GetVertSize();
      vertexClipData = vertexBuffer[0];
    }

    Aligned_WindowTransform(vertexClipData, stride, vertClipLength, windowTransformVec0, windowTransformVec1, mWidth, mHeight);
  }
}

ClipBounds Camera::ClipBoundsCheck(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, const Mat4x4& objectTransform) {
  NamedScopedTimer(ClipBoundsCheck);

  const size_t inIndexSize = indexBuffer.GetIndexSize();

  // Apply the perspective projection transform to all input vertices.
  Aligned_Mat4x4Transform((const float(*)[4])*(mPerspectiveTransform * objectTransform), vertexBuffer[0], vertexBuffer.GetStride(), vertexBuffer.GetVertSize() * vertexBuffer.GetStride());

  // Clip against near plane to avoid things behind camera reappearing.
  // This clip is special because it needs to append clip data and shuffle it back to the beginning.
  // The near clip edge must be > 0, hence we negate the plane.
  ClipTrianglesNearPlane(vertexBuffer, indexBuffer, -mStandardNearPlane);

  if (indexBuffer.GetIndexSize() == 0) {
    return ClipBounds::Outside;
  }
  else if (indexBuffer.GetIndexSize() != inIndexSize) {
    return ClipBounds::ClippedNear;
  }

  // Points at this stage must have Z > 0.
  // If Z < 0, points will re-appear in front of the camera.
  // If Z = 0, divide by 0 occurs and trashes the results.
  Aligned_Vec4Homogenize(vertexBuffer[0], vertexBuffer.GetStride(), vertexBuffer.GetVertSize() * vertexBuffer.GetStride());

  ClipTriangles(vertexBuffer, indexBuffer);
  
  if (indexBuffer.GetClipLength() == 0) {
    return ClipBounds::Outside;
  }
  else if (indexBuffer.GetClipLength() != inIndexSize) {
    return ClipBounds::ClippedNDC;
  }
  else {
    return ClipBounds::Inside;
  }
}

void Camera::OnResize(size_t width, size_t height) {
  mWindowTransform[0][0] = -((float)width);
  mWindowTransform[0][2] = (float)width;
  mWindowTransform[1][1] = (float)height;
  mWindowTransform[1][2] = (float)height;
  mWindowTransform = mWindowTransform * 0.5f;
  mWidth = (float)width;
  mHeight = (float)height;
}
  
}
