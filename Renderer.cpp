﻿#include <cstddef>

#include "AssetLoader.h"
#include "Constants.h"
#include "InputManager.h"
#include "Mat4x4.h"
#include "Quaternion.h"
#include "Renderer.h"
#include "Triangle.h"
#include "UtilMath.h"
#include "ZColor.h"
#include "ZConfig.h"
#include "ZDrawing.h"

namespace ZSharp {
Renderer::Renderer(size_t width, size_t height, size_t stride)
  : mBuffer(width, height, stride)
{
  FileString assetName("C:\\Users\\refik\\Desktop\\backpack.txt");
  LoadModelOBJ(assetName, mModel);

  size_t indexBufSize = 0;
  for (Mesh& mesh : mModel.GetMeshData()) {
    indexBufSize += (mesh.GetTriangleFaceTable().size() * TRI_VERTS);
  }

  mIndexBuffer.Resize(indexBufSize);
  mVertexBuffer.Resize(indexBufSize * TRI_VERTS, TRI_VERTS);

  mCameraPos[0] = 0.0f;
  mCameraPos[1] = 0.0f;
  mCameraPos[2] = 25.0f;

  InputManager& inputManager = InputManager::GetInstance();
  inputManager.Register(this);
}

uint8_t* Renderer::RenderNextFrame() {
  ZConfig& config = ZConfig::GetInstance();

  if (config.SizeChanged(mBuffer.GetWidth(), mBuffer.GetHeight())) {
    mBuffer.Resize();
  }

  InputManager& inputManager = InputManager::GetInstance();
  inputManager.Process();

  mCamera.MoveCamera(mCameraPos);

  mIndexBuffer.Clear();
  mVertexBuffer.Clear();

  mModel.FillBuffers(mVertexBuffer, mIndexBuffer);

  Mat4x4 rotationMatrix;
  rotationMatrix.Identity();
  rotationMatrix.SetRotation(DegreesToRadians(static_cast<float>(mFrameCount)), Mat4x4::Axis::Y);

  if (!mPauseTransforms) {
    mFrameCount += mRotationSpeed;
  }

  if (mFrameCount > 360) {
    mFrameCount = 0;
  }

  mVertexBuffer.ApplyTransform(rotationMatrix);
  
  const ZColor colorRed{ZColors::RED};
  const ZColor colorBlue{ZColors::BLUE};

  mBuffer.Clear(colorBlue);

  mCamera.PerspectiveProjection(mVertexBuffer, mIndexBuffer);

  if (mRenderMode) {
    DrawTrianglesFlat(mBuffer, mVertexBuffer, mIndexBuffer, colorRed);
  }
  else {
    DrawTrianglesWireframe(mBuffer, mVertexBuffer, mIndexBuffer, colorRed);
  }

  return mBuffer.GetBuffer();
}

void Renderer::MoveCamera(Direction direction, const float amount) {
  switch (direction) {
    case Direction::FORWARD:
      mCameraPos[2] -= amount;
      break;
    case Direction::BACK:
      mCameraPos[2] += amount;
      break;
    case Direction::LEFT:
      mCameraPos[0] += amount;
      break;
    case Direction::RIGHT:
      mCameraPos[0] -= amount;
      break;
  }
}

void Renderer::RotateCamera(Mat4x4::Axis direction, const float angleDegrees) {
  Vec3 rotationAxis;
  switch (direction) {
    case Mat4x4::Axis::X:
      rotationAxis[0] = 1.f;
      break;
    case Mat4x4::Axis::Y:
      rotationAxis[1] = 1.f;
      break;
    case Mat4x4::Axis::Z:
      rotationAxis[2] = 1.f;
      break;
  }

  Quaternion quat(DegreesToRadians(angleDegrees), rotationAxis);
  Mat4x4 rotation(quat.GetRotationMatrix());
  mCamera.RotateCamera(rotation);
}

void Renderer::RotateTrackball(Quaternion quat) {
  Mat4x4 rotation(quat.GetRotationMatrix());
  mCamera.RotateCamera(rotation);
}

void Renderer::ChangeSpeed(int64_t amount) {
  if (mRotationSpeed + amount > 10) {
    mRotationSpeed = 10;
  }
  else if (mRotationSpeed + amount < 1) {
    mRotationSpeed = 1;
  }
  else {
    mRotationSpeed += amount;
  }
}

void Renderer::FlipRenderMode() {
  mRenderMode = !mRenderMode;
}

void Renderer::PauseTransforms() {
  mPauseTransforms = !mPauseTransforms;
}

void Renderer::OnKeyDown(uint8_t key) {
  switch (key) {
  case 'P':
    PauseTransforms();
    break;
  case 'R':
    FlipRenderMode();
    break;
  case 'W':
    MoveCamera(ZSharp::Renderer::Direction::FORWARD, 1.0F);
    break;
  case 'S':
    MoveCamera(ZSharp::Renderer::Direction::BACK, 1.0F);
    break;
  case 'A':
    MoveCamera(ZSharp::Renderer::Direction::RIGHT, 1.0F);
    break;
  case 'D':
    MoveCamera(ZSharp::Renderer::Direction::LEFT, 1.0F);
    break;
  case 'Q':
    RotateCamera(Mat4x4::Axis::Y, 1.0F);
    break;
  case 'E':
    RotateCamera(Mat4x4::Axis::Y, -1.0F);
    break;
    // TODO: Come up with a better system for mapping non trivial keys.
  case 0x26: // VK_UP Windows
    ChangeSpeed(1);
    break;
  case 0x28:
    ChangeSpeed(-1);
    break;
  default:
    break;
  }
}

void Renderer::OnKeyUp(uint8_t key) {
  (void)key;
}

void Renderer::OnMouseMove(int32_t oldX, int32_t oldY, int32_t x, int32_t y) {
  Vec3 V1(ProjectClick((float)x, (float)y));
  Vec3 V2(ProjectClick((float)oldX, (float)oldY));
  V1.Normalize();
  V2.Normalize();
  Vec3 N(V1.Cross(V2));
  float theta = acosf(V1 * V2);
  Quaternion quat(DegreesToRadians(theta), N);
  RotateTrackball(quat);
}

Vec3 Renderer::ProjectClick(float x, float y) {
  ZConfig& config = ZConfig::GetInstance();
  int32_t width = (int32_t)config.GetViewportWidth();
  const float radius = (float)width / 2.f;

  bool insideSphere = ((x * x) + (y * y)) < ((radius * radius) / 2.f);
  float z = 0.f;
  if (insideSphere) {
    z = sqrtf((radius * radius) - ((x * x) + (y * y)));
  }
  else {
    z = (((radius * radius) / 2.f) / sqrtf(((x * x) + (y * y))));
  }

  Vec3 result(x, y, z);
  return result;
}

}
