#pragma once

#include <cmath>
#include <cstdint>

#include <array>

#include "Constants.h"
#include "IndexBuffer.h"
#include "Mat2x3.h"
#include "Mat4x4.h"
#include "Triangle.h"
#include "Vec3.h"
#include "VertexBuffer.h"
#include "ZAlgorithm.h"
#include "ZConfig.h"

namespace ZSharp {

class Camera final {
  public:
  Camera() {
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

  Vec3 GetPosition() const {
    return mPosition;
  }

  void MoveCamera(const Vec3& position) {
    mPosition = position;
  }

  void RotateCamera(const Mat4x4& rotationMatrix) {
    Vec4 rotatedVec(mLook);
    rotatedVec = Mat4x4::ApplyTransform(rotationMatrix, rotatedVec);

    Vec4::Homogenize(rotatedVec, 3);
    mLook[0] = rotatedVec[0];
    mLook[1] = rotatedVec[1];
    mLook[2] = rotatedVec[2];

    rotatedVec = mUp;
    rotatedVec = Mat4x4::ApplyTransform(rotationMatrix, rotatedVec);

    Vec4::Homogenize(rotatedVec, 3);
    mUp[0] = rotatedVec[0];
    mUp[1] = rotatedVec[1];
    mUp[2] = rotatedVec[2];
  }

  void PerspectiveProjection(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer) {
    Vec3 w;
    w = mLook * -1.f;
    Vec3::Normalize(w);

    Vec3 v;
    v = mUp - (w * (mUp * w));
    Vec3::Normalize(v);

    Vec3 u;
    u = Vec3::Cross(v, w);

    Mat4x4 translation;
    Mat4x4::Identity(translation);
    Mat4x4::SetTranslation(translation, (mPosition * -1.f));

    Mat4x4 uToE;
    uToE[0] = u;
    uToE[1] = v;
    uToE[2] = w;
    uToE[3][3] = 1.f;

    Mat4x4 scale;
    scale[0][0] = 1.f / (mFarPlane * (std::tanf((mFovHoriz * Constants::PI_OVER_180 / 2.f))));
    scale[1][1] = 1.f / (mFarPlane * (std::tanf((mFovVert * Constants::PI_OVER_180 / 2.f))));
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
    windowTransform[1][1] = static_cast<float>((mHeight * -1.f));
    windowTransform[1][2] = static_cast<float>(mHeight);
    windowTransform = windowTransform * (1.f / 2.f);

    ZAlgorithm::CullBackFacingPrimitives(vertexBuffer, indexBuffer, mPosition);

    std::size_t homogenizedStride = vertexBuffer.GetHomogenizedStride();
    for (std::size_t i = 0; i < vertexBuffer.GetWorkingSize(); i += homogenizedStride) {
      float* vertexData = vertexBuffer.GetInputData(i);
      Vec4& vertexVector = *(reinterpret_cast<Vec4*>(vertexData));
      vertexVector[3] = 1.f;
      vertexVector = Mat4x4::ApplyTransform(unhing, vertexVector);
    }

    for (std::size_t i = 0; i < vertexBuffer.GetWorkingSize(); i += homogenizedStride) {
      float* vertexData = vertexBuffer.GetInputData(i);
      Vec4& vertexVector = *(reinterpret_cast<Vec4*>(vertexData));
      Vec4::Homogenize(vertexVector, 3);
    }

    ClipTriangles(vertexBuffer, indexBuffer);

    std::size_t inputStride = vertexBuffer.GetInputStride();
    for (std::size_t i = 0; i < vertexBuffer.GetClipLength(); i += inputStride) {
      float* vertexData = vertexBuffer.GetClipData(i);
      Vec3& vertexVector = *(reinterpret_cast<Vec3*>(vertexData));
      Vec3::Homogenize(vertexVector, 2);
      vertexVector = Mat2x3::ApplyTransform(windowTransform, vertexVector);
    }
  }

  private:
  Vec3 mPosition;
  Vec3 mLook;
  Vec3 mUp;

  float mNearPlane;
  float mFarPlane;
  float mFovHoriz;
  float mFovVert;

  std::intptr_t mWidth;
  std::intptr_t mHeight;

  void ClipTriangles(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer) {
    Vec3 currentEdge;

    std::size_t inputStride = vertexBuffer.GetHomogenizedStride();
    std::size_t endEBO = indexBuffer.GetWorkingSize();
    for(std::size_t i = 0; i < endEBO; i += Constants::TRI_VERTS) {
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
      numClippedVerts = ZAlgorithm::SutherlandHodgmanClip(clippedVerts, numClippedVerts, currentEdge);

      currentEdge[0] = 0.f;
      currentEdge[1] = 1.f;
      numClippedVerts = ZAlgorithm::SutherlandHodgmanClip(clippedVerts, numClippedVerts, currentEdge);

      currentEdge[0] = -1.f;
      currentEdge[1] = 0.f;
      numClippedVerts = ZAlgorithm::SutherlandHodgmanClip(clippedVerts, numClippedVerts, currentEdge);

      currentEdge[0] = 0.f;
      currentEdge[1] = -1.f;
      numClippedVerts = ZAlgorithm::SutherlandHodgmanClip(clippedVerts, numClippedVerts, currentEdge);

      currentEdge[1] = 0.f;
      currentEdge[2] = -1.f;
      numClippedVerts = ZAlgorithm::SutherlandHodgmanClip(clippedVerts, numClippedVerts, currentEdge);
      Vec3::Clear(currentEdge);

      if(numClippedVerts > 0) {
        std::size_t currentClipIndex = vertexBuffer.GetClipLength() / Constants::TRI_VERTS;
        const float* clippedVertData = reinterpret_cast<const float*>(clippedVerts.data());
        vertexBuffer.AppendClipData(clippedVertData, numClippedVerts * Constants::TRI_VERTS);

        Triangle nextTriangle(currentClipIndex, currentClipIndex + 1, currentClipIndex + 2);
        indexBuffer.AppendClipData(nextTriangle);
        
        for(std::size_t j = 1; j <= numClippedVerts - Constants::TRI_VERTS; ++j) {
          nextTriangle[0] = ((2 * j) % numClippedVerts) + currentClipIndex;

          if(j == numClippedVerts - Constants::TRI_VERTS) {
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
};
}
