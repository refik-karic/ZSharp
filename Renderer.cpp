#include "Renderer.h"

#include "ZBaseTypes.h"

#include "Constants.h"
#include "IndexBuffer.h"
#include "InputManager.h"
#include "Mat4x4.h"
#include "Model.h"
#include "Quaternion.h"
#include "Triangle.h"
#include "UtilMath.h"
#include "VertexBuffer.h"
#include "ZColor.h"
#include "ZConfig.h"
#include "ZDrawing.h"

namespace ZSharp {
Renderer::Renderer(size_t width, size_t height, size_t stride)
  : mBuffer(width, height, stride)
{
  FileString tempModelPath("C:\\Users\\refik\\Desktop\\backpack.txt");
  mWorld.LoadModel(tempModelPath);

  mCamera.MoveCamera(Vec3(0.f, 3.f, 20.f));

  InputManager& inputManager = InputManager::GetInstance();
  inputManager.Register(this);
}

uint8* Renderer::RenderNextFrame() {
  ZConfig& config = ZConfig::GetInstance();

  if (config.SizeChanged(mBuffer.GetWidth(), mBuffer.GetHeight())) {
    mBuffer.Resize();
  }

  InputManager& inputManager = InputManager::GetInstance();
  inputManager.Process();

  Mat4x4 rotationMatrix;
  rotationMatrix.Identity();
  rotationMatrix.SetRotation(DegreesToRadians(static_cast<float>(mFrameCount)), Mat4x4::Axis::Y);

  if (!mPauseTransforms) {
    mFrameCount += mRotationSpeed;
  }

  if (mFrameCount > 360) {
    mFrameCount = 0;
  }

  const ZColor colorRed{ZColors::RED};
  const ZColor colorBlue{ZColors::BLUE};

  mBuffer.Clear(colorBlue);

  for (size_t i = 0; i < mWorld.GetTotalModels(); ++i) {
    Model& model = mWorld.GetModels()[i];
    VertexBuffer& vertexBuffer = mWorld.GetVertexBuffers()[i];
    IndexBuffer& indexBuffer = mWorld.GetIndexBuffers()[i];

    vertexBuffer.Reset();
    indexBuffer.Reset();
    model.FillBuffers(vertexBuffer, indexBuffer);

    vertexBuffer.ApplyTransform(rotationMatrix);
    mCamera.PerspectiveProjection(vertexBuffer, indexBuffer);

    if (mRenderMode) {
      DrawTrianglesFlat(mBuffer, vertexBuffer, indexBuffer, colorRed);
    }
    else {
      DrawTrianglesWireframe(mBuffer, vertexBuffer, indexBuffer, colorRed);
    }
  }

  return mBuffer.GetBuffer();
}

void Renderer::MoveCamera(Direction direction) {
  Vec3 cameraPosition(mCamera.GetPosition());
  Vec3 cameraLook(mCamera.GetLook());

  switch (direction) {
    case Direction::FORWARD:
      cameraPosition = cameraPosition - cameraLook;
      break;
    case Direction::BACK:
      cameraPosition = cameraPosition + cameraLook;
      break;
    case Direction::LEFT:
    {
      Vec3 sideVec(mCamera.GetUp().Cross(cameraLook));
      cameraPosition = cameraPosition + sideVec;
    }
      break;
    case Direction::RIGHT:
    {
      Vec3 sideVec(cameraLook.Cross(mCamera.GetUp()));
      cameraPosition = cameraPosition + sideVec;
    }
      break;
  }

  mCamera.MoveCamera(cameraPosition);
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

void Renderer::ChangeSpeed(int64 amount) {
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

void Renderer::OnKeyDown(uint8 key) {
  switch (key) {
  case 'P':
    PauseTransforms();
    break;
  case 'R':
    FlipRenderMode();
    break;
  case 'W':
    MoveCamera(ZSharp::Renderer::Direction::FORWARD);
    break;
  case 'S':
    MoveCamera(ZSharp::Renderer::Direction::BACK);
    break;
  case 'A':
    MoveCamera(ZSharp::Renderer::Direction::LEFT);
    break;
  case 'D':
    MoveCamera(ZSharp::Renderer::Direction::RIGHT);
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

void Renderer::OnKeyUp(uint8 key) {
  (void)key;
}

void Renderer::OnMouseMove(int32 oldX, int32 oldY, int32 x, int32 y) {
  Vec3 V1(ProjectClick((float)oldX, (float)oldY));
  Vec3 V2(ProjectClick((float)x, (float)y));

  Vec3 normal = V1.Cross(V2);
  V1.Normalize();
  V2.Normalize();

  float theta = acosf((V1 * V2));

  Quaternion quat(DegreesToRadians(theta), normal);
  RotateTrackball(quat);
}

Vec3 Renderer::ProjectClick(float x, float y) {
  const float radius = 1.f;

  ZConfig& config = ZConfig::GetInstance();
  int32 width = (int32)config.GetViewportWidth();
  int32 height = (int32)config.GetViewportHeight();

  float scale = fminf((float)width, (float)height) - 1;

  float newX = ((2 * x) - width + 1) / scale;
  float newY = ((2 * y) - height + 1) / scale;

  bool insideSphere = ((newX * newX) + (newY * newY)) <= ((radius * radius) / 2.f);
  float z = 0.f;
  if (insideSphere) {
    z = sqrtf((radius * radius) - ((newX * newX) - (newY * newY)));
  }
  else {
    z = (((radius * radius) / 2.f) / sqrtf(((newX * newX) + (newY * newY))));
  }

  return Vec3(-newX, newY, z);
}

}
