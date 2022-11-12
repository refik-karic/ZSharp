#include "GameInstance.h"

#include "Constants.h"
#include "InputManager.h"
#include "UtilMath.h"
#include "ZConfig.h"

#include <cmath>

namespace ZSharp {

GameInstance::GameInstance() {

}

GameInstance::~GameInstance() {
  InputManager& inputManager = InputManager::GetInstance();
  inputManager.OnKeyDownDelegate.Remove(Delegate<uint8>::FromMember<GameInstance, &GameInstance::OnKeyDown>(this));
  inputManager.OnKeyUpDelegate.Remove(Delegate<uint8>::FromMember<GameInstance, &GameInstance::OnKeyUp>(this));
  inputManager.OnMouseMoveDelegate.Remove(Delegate<int32, int32, int32, int32>::FromMember<GameInstance, &GameInstance::OnMouseMove>(this));
}

void GameInstance::LoadAssets() {
  ZConfig& config = ZConfig::GetInstance();
  if (!config.GetAssetPath().GetAbsolutePath().IsEmpty()) {
    for (const String& asset : config.GetAssets()) {
      FileString filePath(config.GetAssetPath().GetAbsolutePath());
      filePath.SetFilename(asset);
      mWorld.LoadModel(filePath);
    }
  }
}

void GameInstance::MoveCamera(Direction direction) {
  Vec3 cameraLook(mCamera.GetLook());

  switch (direction) {
  case Direction::FORWARD:
    mCamera.Position() = mCamera.Position() - cameraLook;
    break;
  case Direction::BACK:
    mCamera.Position() = mCamera.Position() + cameraLook;
    break;
  case Direction::LEFT:
  {
    Vec3 sideVec(mCamera.GetUp().Cross(cameraLook));
    mCamera.Position() = mCamera.Position() + sideVec;
  }
  break;
  case Direction::RIGHT:
  {
    Vec3 sideVec(cameraLook.Cross(mCamera.GetUp()));
    mCamera.Position() = mCamera.Position() + sideVec;
  }
  break;
  }
}

void GameInstance::RotateCamera(Mat4x4::Axis direction, const float angleDegrees) {
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

void GameInstance::RotateTrackball(Quaternion quat) {
  Mat4x4 rotation(quat.GetRotationMatrix());
  mCamera.RotateCamera(rotation);
}

void GameInstance::ChangeSpeed(int64 amount) {
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

void GameInstance::PauseTransforms() {
  mPauseTransforms = !mPauseTransforms;
}

void GameInstance::Initialize() {
  mRenderer.Initialize();

  LoadAssets();

#if 0
  {
    Vec4 v1(-5.f, 0.f, 0.f, 1.f);
    Vec4 v2(0.f, 5.f, 0.f, 1.f);
    Vec4 v3(5.f, 0.f, 0.f, 1.f);
    mWorld.DebugLoadTriangle(v1, v2, v3);
  }
#endif

  mCamera.Position() = Vec3(0.f, 0.f, 30.f);

  InputManager& inputManager = InputManager::GetInstance();
  inputManager.OnKeyDownDelegate.Add(Delegate<uint8>::FromMember<GameInstance, &GameInstance::OnKeyDown>(this));
  inputManager.OnKeyUpDelegate.Add(Delegate<uint8>::FromMember<GameInstance, &GameInstance::OnKeyUp>(this));
  inputManager.OnMouseMoveDelegate.Add(Delegate<int32, int32, int32, int32>::FromMember<GameInstance, &GameInstance::OnMouseMove>(this));
}

void GameInstance::Tick() {
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

  for (Model& model : mWorld.GetModels()) {
    model.SetRotation(rotationMatrix);
  }

  mRenderer.RenderNextFrame(mWorld, mCamera);
}

uint8* GameInstance::GetCurrentFrame() {
  return mRenderer.GetFrame();
}

void GameInstance::OnKeyDown(uint8 key) {
  switch (key) {
  case 'P':
    PauseTransforms();
    break;
  case 'R':
    mRenderer.FlipRenderMode();
    break;
  case 'W':
    MoveCamera(ZSharp::GameInstance::Direction::FORWARD);
    break;
  case 'S':
    MoveCamera(ZSharp::GameInstance::Direction::BACK);
    break;
  case 'A':
    MoveCamera(ZSharp::GameInstance::Direction::LEFT);
    break;
  case 'D':
    MoveCamera(ZSharp::GameInstance::Direction::RIGHT);
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

void GameInstance::OnKeyUp(uint8 key) {
  (void)key;
}

void GameInstance::OnMouseMove(int32 oldX, int32 oldY, int32 x, int32 y) {
  Vec3 V1(ProjectClick((float)oldX, (float)oldY));
  Vec3 V2(ProjectClick((float)x, (float)y));

  Vec3 normal = V1.Cross(V2);
  V1.Normalize();
  V2.Normalize();

  float theta = acosf((V1 * V2));

  Quaternion quat(DegreesToRadians(theta), normal);
  RotateTrackball(quat);
}

Vec3 GameInstance::ProjectClick(float x, float y) {
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
