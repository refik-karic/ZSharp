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

template <typename T>
class Camera final {
  public:
  Camera() {
    ZConfig& config = ZConfig::GetInstance();
    mWidth = config.GetViewportWidth();
    mHeight = config.GetViewportHeight();

    mLook[2] = static_cast<T>(-1);
    mUp[1] = static_cast<T>(-1);

    mFovHoriz = static_cast<T>(90);
    mFovVert = static_cast<T>(90);

    mNearPlane = static_cast<T>(10);
    mFarPlane = static_cast<T>(100);
  }

  Vec3<T> GetPosition() const {
    return mPosition;
  }

  void MoveCamera(const Vec3<T>& position) {
    mPosition = position;
  }

  void RotateCamera(const Mat4x4<T>& rotationMatrix) {
    Vec4<T> rotatedVec(mLook);
    rotatedVec = Mat4x4<T>::ApplyTransform(rotationMatrix, rotatedVec);

    Vec4<T>::Homogenize(rotatedVec, 3);
    mLook[0] = rotatedVec[0];
    mLook[1] = rotatedVec[1];
    mLook[2] = rotatedVec[2];

    rotatedVec = mUp;
    rotatedVec = Mat4x4<T>::ApplyTransform(rotationMatrix, rotatedVec);

    Vec4<T>::Homogenize(rotatedVec, 3);
    mUp[0] = rotatedVec[0];
    mUp[1] = rotatedVec[1];
    mUp[2] = rotatedVec[2];
  }

  void PerspectiveProjection(VertexBuffer<T>& vertexBuffer, IndexBuffer& indexBuffer) {
    Vec3<T> w;
    w = mLook * static_cast<T>(-1);
    Vec3<T>::Normalize(w);

    Vec3<T> v;
    v = mUp - (w * (mUp * w));
    Vec3<T>::Normalize(v);

    Vec3<T> u;
    u = Vec3<T>::Cross(v, w);

    Mat4x4<T> translation;
    Mat4x4<T>::Identity(translation);
    Mat4x4<T>::SetTranslation(translation, (mPosition * static_cast<T>(-1)));

    Mat4x4<T> uToE;
    uToE[0] = u;
    uToE[1] = v;
    uToE[2] = w;
    uToE[3][3] = static_cast<T>(1);

    Mat4x4<T> scale;
    scale[0][0] = static_cast<T>(1) / (mFarPlane * (std::tan((mFovHoriz * static_cast<T>(Constants::PI_OVER_180) / static_cast<T>(2)))));
    scale[1][1] = static_cast<T>(1) / (mFarPlane * (std::tan((mFovVert * static_cast<T>(Constants::PI_OVER_180) / static_cast<T>(2)))));
    scale[2][2] = static_cast<T>(1) / mFarPlane;
    scale[3][3] = static_cast<T>(1);

    Mat4x4<T> unhing;
    unhing[0][0] = mFarPlane - mNearPlane;
    unhing[1][1] = mFarPlane - mNearPlane;
    unhing[2][2] = mFarPlane;
    unhing[2][3] = mNearPlane;
    unhing[3][2] = (mFarPlane - mNearPlane) * static_cast<T>(-1);

    unhing = unhing * (scale * (uToE * translation));

    Mat2x3<T> windowTransform;
    windowTransform[0][0] = static_cast<T>(mWidth);
    windowTransform[0][2] = static_cast<T>(mWidth);
    windowTransform[1][1] = static_cast<T>((mHeight * static_cast<T>(-1)));
    windowTransform[1][2] = static_cast<T>(mHeight);
    windowTransform = windowTransform * (static_cast<T>(1.0 / 2.0));

    ZAlgorithm<T>::CullBackFacingPrimitives(vertexBuffer, indexBuffer, mPosition);

    std::size_t homogenizedStride = vertexBuffer.GetHomogenizedStride();
    for (std::size_t i = 0; i < vertexBuffer.GetWorkingSize(); i += homogenizedStride) {
      T* vertexData = vertexBuffer.GetInputData(i);
      Vec4<T>& vertexVector = *(reinterpret_cast<Vec4<T>*>(vertexData));
      vertexVector[3] = static_cast<T>(1);
      vertexVector = Mat4x4<T>::ApplyTransform(unhing, vertexVector);
    }

    for (std::size_t i = 0; i < vertexBuffer.GetWorkingSize(); i += homogenizedStride) {
      T* vertexData = vertexBuffer.GetInputData(i);
      Vec4<T>& vertexVector = *(reinterpret_cast<Vec4<T>*>(vertexData));
      Vec4<T>::Homogenize(vertexVector, 3);
    }

    ClipTriangles(vertexBuffer, indexBuffer);

    std::size_t inputStride = vertexBuffer.GetInputStride();
    for (std::size_t i = 0; i < vertexBuffer.GetClipLength(); i += inputStride) {
      T* vertexData = vertexBuffer.GetClipData(i);
      Vec3<T>& vertexVector = *(reinterpret_cast<Vec3<T>*>(vertexData));
      Vec3<T>::Homogenize(vertexVector, 2);
      vertexVector = Mat2x3<T>::ApplyTransform(windowTransform, vertexVector);
    }
  }

  private:
  Vec3<T> mPosition;
  Vec3<T> mLook;
  Vec3<T> mUp;

  T mNearPlane;
  T mFarPlane;
  T mFovHoriz;
  T mFovVert;

  std::intptr_t mWidth;
  std::intptr_t mHeight;

  void ClipTriangles(VertexBuffer<T>& vertexBuffer, IndexBuffer& indexBuffer) {
    Vec3<T> currentEdge;

    std::size_t inputStride = vertexBuffer.GetHomogenizedStride();
    std::size_t endEBO = indexBuffer.GetWorkingSize();
    for(std::size_t i = 0; i < endEBO; i += Constants::TRI_VERTS) {
      T* v1 = vertexBuffer.GetInputData(indexBuffer[i], inputStride);
      T* v2 = vertexBuffer.GetInputData(indexBuffer[i + 1], inputStride);
      T* v3 = vertexBuffer.GetInputData(indexBuffer[i + 2], inputStride);
      std::size_t numClippedVerts = 3;
      std::array<Vec3<T>, 6> clippedVerts{
        *(reinterpret_cast<Vec3<T>*>(v1)),
        *(reinterpret_cast<Vec3<T>*>(v2)),
        *(reinterpret_cast<Vec3<T>*>(v3))
      };

      currentEdge[0] = static_cast<T>(1);
      numClippedVerts = ZAlgorithm<T>::SutherlandHodgmanClip(clippedVerts, numClippedVerts, currentEdge);

      currentEdge[0] = static_cast<T>(0);
      currentEdge[1] = static_cast<T>(1);
      numClippedVerts = ZAlgorithm<T>::SutherlandHodgmanClip(clippedVerts, numClippedVerts, currentEdge);

      currentEdge[0] = static_cast<T>(-1);
      currentEdge[1] = static_cast<T>(0);
      numClippedVerts = ZAlgorithm<T>::SutherlandHodgmanClip(clippedVerts, numClippedVerts, currentEdge);

      currentEdge[0] = static_cast<T>(0);
      currentEdge[1] = static_cast<T>(-1);
      numClippedVerts = ZAlgorithm<T>::SutherlandHodgmanClip(clippedVerts, numClippedVerts, currentEdge);

      currentEdge[1] = static_cast<T>(0);
      currentEdge[2] = static_cast<T>(-1);
      numClippedVerts = ZAlgorithm<T>::SutherlandHodgmanClip(clippedVerts, numClippedVerts, currentEdge);
      Vec3<T>::Clear(currentEdge);

      if(numClippedVerts > 0) {
        std::size_t currentClipIndex = vertexBuffer.GetClipLength() / Constants::TRI_VERTS;
        const T* clippedVertData = reinterpret_cast<const T*>(clippedVerts.data());
        vertexBuffer.AppendClipData(clippedVertData, numClippedVerts * Constants::TRI_VERTS);

        Triangle<T> nextTriangle(currentClipIndex, currentClipIndex + 1, currentClipIndex + 2);
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
