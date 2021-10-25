#include "Camera.h"

#include <cmath>

#include <array>

#include "Constants.h"
#include "Mat2x3.h"
#include "Triangle.h"
#include "ZAlgorithm.h"
#include "ZConfig.h"

#define DEBUG_CLIPPING 0

namespace ZSharp {
Camera::Camera() {
  ZConfig& config = ZConfig::GetInstance();
  mWidth = config.GetViewportWidth();
  mHeight = config.GetViewportHeight();

  mLook[2] = -1.f;
  mUp[1] = -1.f;

  mFovHoriz = 90.f;
  mFovVert = 90.f;

  mNearPlane = 10.f;
  mFarPlane = 100.f;
}

Vec3 Camera::GetPosition() const {
  return mPosition;
}

void Camera::MoveCamera(const Vec3& position) {
  mPosition = position;
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
  Vec3 w(-mLook);
  w.Normalize();

  Vec3 v;
  v = mUp - (w * (mUp * w));
  v.Normalize();

  Vec3 u;
  u = v.Cross(w);

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

  mWidth = ZConfig::GetInstance().GetViewportWidth();
  mHeight = ZConfig::GetInstance().GetViewportHeight();

  Mat2x3 windowTransform;
  windowTransform[0][0] = static_cast<float>(mWidth);
  windowTransform[0][2] = static_cast<float>(mWidth);
  windowTransform[1][1] = -static_cast<float>(mHeight);
  windowTransform[1][2] = static_cast<float>(mHeight);
  windowTransform = windowTransform * (1.f / 2.f);

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
    vertexVector = windowTransform.ApplyTransform(vertexVector);
  }
}

void Camera::ClipTriangles(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer) {
  static const size_t MAX_OUT_CLIP_VERTS = 6;

  Vec3 currentEdge;

  for (size_t i = 0; i < indexBuffer.GetIndexSize(); i += TRI_VERTS) {
    Vec3* v1 = reinterpret_cast<Vec3*>(vertexBuffer[indexBuffer[i]]);
    Vec3* v2 = reinterpret_cast<Vec3*>(vertexBuffer[indexBuffer[i + 1]]);
    Vec3* v3 = reinterpret_cast<Vec3*>(vertexBuffer[indexBuffer[i + 2]]);
    size_t numClippedVerts = 3;
    std::array<Vec3, MAX_OUT_CLIP_VERTS> clippedVerts{*v1, *v2, *v3};

#if DEBUG_CLIPPING
    size_t currentClipIndex = vertexBuffer.GetClipLength() / TRI_VERTS;

    std::array<Vec4, MAX_OUT_CLIP_VERTS> tempClippedVerts;
    for (size_t j = 0; j < numClippedVerts; ++j) {
      tempClippedVerts[j] = clippedVerts[j];
    }

    vertexBuffer.AppendClipData(reinterpret_cast<const float*>(tempClippedVerts.data()), 3);
    Triangle nextTriangle(currentClipIndex, currentClipIndex + 1, currentClipIndex + 2);
    indexBuffer.AppendClipData(nextTriangle);
#else
    currentEdge[0] = 1.f;
    numClippedVerts = SutherlandHodgmanClip(clippedVerts, numClippedVerts, currentEdge);

    currentEdge[0] = 0.f;
    currentEdge[1] = 1.f;
    numClippedVerts = SutherlandHodgmanClip(clippedVerts, numClippedVerts, currentEdge);

    currentEdge[0] = -1.f;
    currentEdge[1] = 0.f;
    numClippedVerts = SutherlandHodgmanClip(clippedVerts, numClippedVerts, currentEdge);

    currentEdge[0] = 0.f;
    currentEdge[1] = -1.f;
    numClippedVerts = SutherlandHodgmanClip(clippedVerts, numClippedVerts, currentEdge);

    currentEdge[1] = 0.f;
    currentEdge[2] = -1.f;
    numClippedVerts = SutherlandHodgmanClip(clippedVerts, numClippedVerts, currentEdge);
    currentEdge.Clear();

    if (numClippedVerts > 0) {
      size_t currentClipIndex = vertexBuffer.GetClipLength() / TRI_VERTS;

      std::array<Vec4, 6> tempClippedVerts;
      for (size_t j = 0; j < numClippedVerts; ++j) {
        tempClippedVerts[j] = clippedVerts[j];
      }

      const float* clippedVertData = reinterpret_cast<const float*>(tempClippedVerts.data());
      vertexBuffer.AppendClipData(clippedVertData, numClippedVerts);

      Triangle nextTriangle(currentClipIndex, currentClipIndex + 1, currentClipIndex + 2);
      indexBuffer.AppendClipData(nextTriangle);

      for (size_t j = 1; j <= numClippedVerts - TRI_VERTS; ++j) {
        nextTriangle[0] = ((2 * j) % numClippedVerts) + currentClipIndex;

        if (j == numClippedVerts - TRI_VERTS) {
          size_t secondPos = (((numClippedVerts - 4) >> 1) + 1);
          size_t thirdPos = ((numClippedVerts - TRI_VERTS) + 1);

          nextTriangle[1] = (((2 * j) + secondPos) % numClippedVerts) + currentClipIndex;
          nextTriangle[2] = (((2 * j) + thirdPos) % numClippedVerts) + currentClipIndex;
        }
        else {
          nextTriangle[1] = (((2 * j) + 1) % numClippedVerts) + currentClipIndex;
          nextTriangle[2] = (((2 * j) + 2) % numClippedVerts) + currentClipIndex;
        }

        indexBuffer.AppendClipData(nextTriangle);
      }
    }
#endif
  }
}
}
