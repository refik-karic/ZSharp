#include <cstddef>

#include <filesystem>

#include "AssetLoader.h"
#include "Constants.h"
#include "InputManager.h"
#include "Mat4x4.h"
#include "Renderer.h"
#include "Triangle.h"
#include "UtilMath.h"
#include "ZColor.h"
#include "ZDrawing.h"

namespace ZSharp {
Renderer::Renderer(std::size_t width, std::size_t height, std::size_t stride)
  : mBuffer(width, height, stride)
{
  std::filesystem::path assetToLoad = std::filesystem::current_path().parent_path().append("pyramids.json");
  AssetLoader::LoadModel<float>(assetToLoad.string().c_str(), mModel);
  
  std::size_t indexBufSize = 0;
  for (Mesh<float>& mesh : mModel.GetMeshData()) {
    indexBufSize += (mesh.GetTriangleFaceTable().size() * Constants::TRI_VERTS);
  }

  mIndexBuffer = std::make_shared<IndexBuffer>(indexBufSize);
  mVertexBuffer = std::make_shared<VertexBuffer<float>>(indexBufSize * Constants::TRI_VERTS, Constants::TRI_VERTS);

  mCameraPos[0] = 0.0f;
  mCameraPos[1] = 0.0f;
  mCameraPos[2] = 35.0f;

  InputManager* inputManager = InputManager::GetInstance();
  inputManager->Register(this);
}

std::uint8_t* Renderer::RenderNextFrame() {
  InputManager* inputManager = InputManager::GetInstance();
  inputManager->Process();

  mCamera.MoveCamera(mCameraPos);

  mIndexBuffer->Clear();
  mVertexBuffer->Clear();

  mModel.FillBuffers(*mVertexBuffer, *mIndexBuffer);

  Mat4x4<float> rotationMatrix;
  Mat4x4<float>::Identity(rotationMatrix);
  Mat4x4<float>::SetRotation(rotationMatrix,
                          static_cast<float>(DegreesToRadians(static_cast<double>(mFrameCount))),
                          Mat4x4<float>::Axis::Y);

  if (!mPauseTransforms) {
    mFrameCount += mRotationSpeed;
  }

  if (mFrameCount > 360) {
    mFrameCount = 0;
  }

  mVertexBuffer->ApplyTransform(rotationMatrix);
  
  ZColor colorRed{ZColors::RED};
  ZColor colorBlue{ZColors::BLUE};

  mBuffer.Clear(colorBlue);

  mCamera.PerspectiveProjection(*mVertexBuffer, *mIndexBuffer);

  if (mRenderMode) {
    ZDrawing::DrawTrianglesFlat(mBuffer, *mVertexBuffer, *mIndexBuffer, colorRed);
  }
  else {
    ZDrawing::DrawTrianglesWireframe(mBuffer, *mVertexBuffer, *mIndexBuffer, colorRed);
  }

  return mBuffer.GetBuffer();
}

void Renderer::MoveCamera(Direction direction, float amount) {
  switch (direction) {
    case Direction::UP:
      mCameraPos[1] -= amount;
      break;
    case Direction::DOWN:
      mCameraPos[1] += amount;
      break;
    case Direction::LEFT:
      mCameraPos[0] -= amount;
      break;
    case Direction::RIGHT:
      mCameraPos[0] += amount;
      break;
  }
}

void Renderer::RotateCamera(Mat4x4<float>::Axis direction, float angleDegrees) {
  Mat4x4<float> rotationMatrix;
  Mat4x4<float>::Identity(rotationMatrix);
  Mat4x4<float>::SetRotation(rotationMatrix,
    static_cast<float>(DegreesToRadians(static_cast<double>(angleDegrees))),
    direction);

  mCamera.RotateCamera(rotationMatrix);
}

void Renderer::ChangeSpeed(std::int64_t amount) {
  if(mRotationSpeed + amount > 10) {
    mRotationSpeed = 10;
  }
  else if(mRotationSpeed + amount < 1) {
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

void Renderer::OnKeyDown(std::uint8_t key) {
  switch (key) {
    case 'P':
      PauseTransforms();
      break;
    case 'R':
      FlipRenderMode();
      break;
    case 'W':
      MoveCamera(ZSharp::Renderer::Direction::UP, 1.0F);
      break;
    case 'S':
      MoveCamera(ZSharp::Renderer::Direction::DOWN, 1.0F);
      break;
    case 'A':
      MoveCamera(ZSharp::Renderer::Direction::RIGHT, 1.0F);
      break;
    case 'D':
      MoveCamera(ZSharp::Renderer::Direction::LEFT, 1.0F);
      break;
    case 'Q':
      RotateCamera(Mat4x4<float>::Axis::Y, 1.0F);
      break;
    case 'E':
      RotateCamera(Mat4x4<float>::Axis::Y, -1.0F);
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

void Renderer::OnKeyUp(std::uint8_t key) {
  (void)key;
}

}
