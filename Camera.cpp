#include "Camera.h"

#include <cmath>

#include "CommonMath.h"
#include "Constants.h"
#include "Win32PlatformApplication.h"
#include "ZAlgorithm.h"
#include "ZConfig.h"
#include "PlatformIntrinsics.h"
#include "ScopedTimer.h"

#define DISABLE_BACKFACE_CULLING 0
#define ROTATE_CAMERA_WORLD_CENTER 1

namespace ZSharp {
Camera::Camera() {
  mLook[2] = -1.f;
  mUp[1] = 1.f;

  mFovHoriz = 70.f;
  mFovVert = 45.f;

  //mNearPlane = 0.02f;
  //mFarPlane = 1000.f;

  mNearPlane = 10.f;
  mFarPlane = 100.f;

  const ZConfig& config = ZConfig::Get();
  OnResize(config.GetViewportWidth().Value(), config.GetViewportHeight().Value());

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

  Mat4x4 unhing;
  unhing[0][0] = standardFarPlane - mStandardNearPlane;
  unhing[1][1] = standardFarPlane - mStandardNearPlane;
  unhing[2][2] = standardFarPlane;
  unhing[2][3] = mStandardNearPlane;
  unhing[3][2] = -(standardFarPlane - mStandardNearPlane);

  mPerspectiveTransform = (unhing * (scale * (uToE * translation)));
}

void Camera::PerspectiveProjection(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, ClipBounds clipBounds) {
  NamedScopedTimer(PerspectiveProjection);

  if (clipBounds == ClipBounds::Outside) {
    return;
  }

#if !DISABLE_BACKFACE_CULLING
  CullBackFacingPrimitives(vertexBuffer, indexBuffer, mPosition);
#endif

  const int32 stride = vertexBuffer.GetStride();

  // Apply the perspective projection transform to all input vertices.
  Aligned_Mat4x4Transform((const float(*)[4]) * mPerspectiveTransform, vertexBuffer[0], stride, vertexBuffer.GetVertSize() * stride);

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

  if (clipBounds == ClipBounds::Inside) {
    vertexBuffer.AppendClipData(vertexBuffer[0], vertexBuffer.GetVertSize() * stride * sizeof(float), vertexBuffer.GetVertSize());
    indexBuffer.AppendClipData(indexBuffer.GetInputData(), indexBuffer.GetIndexSize());
  }
  else {
    ClipTriangles(vertexBuffer, indexBuffer);
  }

  const int32 vertClipLength = vertexBuffer.GetClipLength();
  const float* windowTransformVec0 = *mWindowTransform[0];
  const float* windowTransformVec1 = *mWindowTransform[1];
  float* vertexClipData = vertexBuffer.GetClipData(0);

  for (int32 i = 0; i < vertClipLength; ++i) {
    float* vertexData = vertexClipData + (i * stride);

    const float perspectiveZ = vertexData[3];
    const float invPerspectiveZ = 1.f / vertexData[3];

    // Homogenize
    const float invDivisor = 1.f / vertexData[2];
    vertexData[0] *= invDivisor;
    vertexData[1] *= invDivisor;
    vertexData[2] *= invDivisor;

    // Apply Window transform.
    const float dotX = Dot3(vertexData, windowTransformVec0);
    const float dotY = Dot3(vertexData, windowTransformVec1);

    vertexData[0] = dotX;
    vertexData[1] = dotY;

    // Store perspective Z and inverse Z for each vertex in the clip buffer.
    // Prevents us from having to calculate this at a later point in the drawing code.
    vertexData[2] = perspectiveZ;
    vertexData[3] = invPerspectiveZ;

    for (size_t j = 4; j < stride; ++j) {
      vertexData[j] *= invPerspectiveZ;
    }
  }
}

ClipBounds Camera::ClipBoundsCheck(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer) {
  NamedScopedTimer(ClipBoundsCheck);

  const size_t inIndexSize = indexBuffer.GetIndexSize();

  // Apply the perspective projection transform to all input vertices.
  Aligned_Mat4x4Transform((const float(*)[4])* mPerspectiveTransform, vertexBuffer[0], vertexBuffer.GetStride(), vertexBuffer.GetVertSize() * vertexBuffer.GetStride());

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
}
  
}
