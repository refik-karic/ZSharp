#include "Camera.h"

#include <cmath>

#include <array>

#include "Constants.h"
#include "Mat2x3.h"
#include "Triangle.h"
#include "ZAlgorithm.h"
#include "ZConfig.h"

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

  rotatedVec.Homogenize(3);
  mLook[0] = rotatedVec[0];
  mLook[1] = rotatedVec[1];
  mLook[2] = rotatedVec[2];

  rotatedVec = mUp;
  rotatedVec = rotationMatrix.ApplyTransform(rotatedVec);

  rotatedVec.Homogenize(3);
  mUp[0] = rotatedVec[0];
  mUp[1] = rotatedVec[1];
  mUp[2] = rotatedVec[2];
}

void Camera::PerspectiveProjection(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer) {
  Vec3 w;
  w = mLook * -1.f;
  w.Normalize();

  Vec3 v;
  v = mUp - (w * (mUp * w));
  v.Normalize();

  Vec3 u;
  u = v.Cross(w);

  Mat4x4 translation;
  translation.Identity();
  translation.SetTranslation(mPosition * -1.f);

  Mat4x4 uToE;
  uToE[0] = u;
  uToE[1] = v;
  uToE[2] = w;
  uToE[3][3] = 1.f;

  Mat4x4 scale;
  scale[0][0] = 1.f / (mFarPlane * (tanf((mFovHoriz * (Constants::PI_OVER_180 / 2.f)))));
  scale[1][1] = 1.f / (mFarPlane * (tanf((mFovVert * (Constants::PI_OVER_180 / 2.f)))));
  scale[2][2] = 1.f / mFarPlane;
  scale[3][3] = 1.f;

  Mat4x4 unhing;
  unhing[0][0] = mFarPlane - mNearPlane;
  unhing[1][1] = mFarPlane - mNearPlane;
  unhing[2][2] = mFarPlane;
  unhing[2][3] = mNearPlane;
  unhing[3][2] = (mFarPlane - mNearPlane) * -1.f;

  unhing = unhing * (scale * (uToE * translation));

  Mat2x3 windowTransform;
  windowTransform[0][0] = static_cast<float>(mWidth);
  windowTransform[0][2] = static_cast<float>(mWidth);
  windowTransform[1][1] = static_cast<float>(mHeight) * -1.f;
  windowTransform[1][2] = static_cast<float>(mHeight);
  windowTransform = windowTransform * (1.f / 2.f);

  CullBackFacingPrimitives(vertexBuffer, indexBuffer, mPosition);

  std::size_t homogenizedStride = vertexBuffer.GetHomogenizedStride();
  for (std::size_t i = 0; i < vertexBuffer.GetWorkingSize(); i += homogenizedStride) {
    float* vertexData = vertexBuffer.GetInputData(i);
    Vec4& vertexVector = *(reinterpret_cast<Vec4*>(vertexData));
    vertexVector[3] = 1.f;
    vertexVector = unhing.ApplyTransform(vertexVector);
  }

  for (std::size_t i = 0; i < vertexBuffer.GetWorkingSize(); i += homogenizedStride) {
    float* vertexData = vertexBuffer.GetInputData(i);
    Vec4& vertexVector = *(reinterpret_cast<Vec4*>(vertexData));
    vertexVector.Homogenize(3);
  }

  ClipTriangles(vertexBuffer, indexBuffer);

  std::size_t inputStride = vertexBuffer.GetInputStride();
  for (std::size_t i = 0; i < vertexBuffer.GetClipLength(); i += inputStride) {
    float* vertexData = vertexBuffer.GetClipData(i);
    Vec3& vertexVector = *(reinterpret_cast<Vec3*>(vertexData));
    vertexVector.Homogenize(2);
    vertexVector = windowTransform.ApplyTransform(vertexVector);
  }
}

void Camera::ClipTriangles(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer) {
  Vec3 currentEdge;

  std::size_t inputStride = vertexBuffer.GetHomogenizedStride();
  std::size_t endEBO = indexBuffer.GetWorkingSize();
  for (std::size_t i = 0; i < endEBO; i += Constants::TRI_VERTS) {
    float* v1 = vertexBuffer.GetInputData(indexBuffer[i], inputStride);
    float* v2 = vertexBuffer.GetInputData(indexBuffer[i + 1], inputStride);
    float* v3 = vertexBuffer.GetInputData(indexBuffer[i + 2], inputStride);
    std::size_t numClippedVerts = 3;
    std::array<Vec3, 6> clippedVerts{
      *(reinterpret_cast<Vec3*>(v1)),
      *(reinterpret_cast<Vec3*>(v2)),
      *(reinterpret_cast<Vec3*>(v3))
    };

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
      std::size_t currentClipIndex = vertexBuffer.GetClipLength() / Constants::TRI_VERTS;
      const float* clippedVertData = reinterpret_cast<const float*>(clippedVerts.data());
      vertexBuffer.AppendClipData(clippedVertData, numClippedVerts * Constants::TRI_VERTS);

      Triangle nextTriangle(currentClipIndex, currentClipIndex + 1, currentClipIndex + 2);
      indexBuffer.AppendClipData(nextTriangle);

      for (std::size_t j = 1; j <= numClippedVerts - Constants::TRI_VERTS; ++j) {
        nextTriangle[0] = ((2 * j) % numClippedVerts) + currentClipIndex;

        if (j == numClippedVerts - Constants::TRI_VERTS) {
          std::size_t secondPos = (((numClippedVerts - 4) >> 1) + 1);
          std::size_t thirdPos = ((numClippedVerts - Constants::TRI_VERTS) + 1);

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
  }
}
}
