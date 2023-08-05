#include "GameInstance.h"

// TODO: Clean this up
#include "Constants.h"
#include "InputManager.h"
#include "CommonMath.h"
#include "Logger.h"
#include "ScopedTimer.h"
#include "ShadingMode.h"
#include "ZConfig.h"
#include "ZString.h"
#include "PlatformTime.h"
#include "PlatformMemory.h"
#include "Constants.h"

#include "PNG.h"
#include "PlatformFile.h"
#include "PlatformMemory.h"
#include "PlatformAudio.h"

#include "Bundle.h"

#include "MP3.h"

#include <cmath>

#define DEBUG_AUDIO 1
#define DEBUG_TRIANGLE 0
#define DEBUG_TRIANGLE_TEXTURE 1
#define DISABLE_DEBUG_TRANSFORMS 1

namespace ZSharp {

GameInstance::GameInstance() {

}

GameInstance::~GameInstance() {
  InputManager& inputManager = InputManager::GetInstance();
  inputManager.OnKeyDownDelegate.Remove(Delegate<uint8>::FromMember<GameInstance, &GameInstance::OnKeyDown>(this));
  inputManager.OnKeyUpDelegate.Remove(Delegate<uint8>::FromMember<GameInstance, &GameInstance::OnKeyUp>(this));
  inputManager.OnMouseMoveDelegate.Remove(Delegate<int32, int32, int32, int32>::FromMember<GameInstance, &GameInstance::OnMouseMove>(this));

  if (mAmbientTrack.data != nullptr) {
    PlatformFree(mAmbientTrack.data);
  }

  if (mAudioDevice != nullptr) {
    PlatformReleaseAudioDevice(mAudioDevice);
  }
}

void GameInstance::LoadAssets() {
#if DEBUG_AUDIO
  FileString audioPath(PlatformGetUserDesktopPath());
  audioPath.SetFilename("AmbientTest.mp3");
  MP3 audioFile(audioPath);

  mAmbientTrack = audioFile.Decompress();
  if (mAmbientTrack.data != nullptr) {
    // TODO: How big of an audio buffer do we actually need if we can fill it at ~1ms intervals?
    mAudioDevice = PlatformInitializeAudioDevice(mAmbientTrack.samplesPerSecond, mAmbientTrack.channels, 100);
    ZAssert(mAudioDevice != nullptr);
  }
#endif

#if DEBUG_TRIANGLE
  const float X = 5.f;
  const float Y = 5.f;
  const float Z = 0.f;
  const float W = 1.f;
  const float v1[]{ -X, 0.f, Z, W, 0.f, 1.f, 0.f };
  const float v2[]{ 0.f, Y, Z, W, 1.0f, 0.f, 0.f };
  const float v3[]{ X, 0.f, Z, W, 0.0f, 0.f, 1.f };

  ShadingModeOrder order;
  ShadingMode mode(ShadingModes::RGB, 3);
  order.PushBack(mode);

  mWorld.DebugLoadTriangle(v1, v2, v3, order, 7);
#elif DEBUG_TRIANGLE_TEXTURE
  const float X = 5.f;
  const float Y = 5.f;
  const float Z = 0.f;
  const float W = 1.f;
  const float v1[]{ -X, 0.f, Z, W, 0.f, 1.f };
  const float v2[]{ 0.f, Y, Z, W, 0.5f, 0.f };
  const float v3[]{ X, 0.f, Z, W, 1.0f, 1.f };

  ShadingModeOrder order;
  ShadingMode mode(ShadingModes::UV, 2);
  order.PushBack(mode);

  mWorld.DebugLoadTriangle(v1, v2, v3, order, 6);
#else
  ZConfig& config = ZConfig::GetInstance();
  if (!config.GetAssetPath().GetAbsolutePath().IsEmpty()) {
    mWorld.LoadModels();
  }
  else {
    Logger::Log(LogCategory::Warning, "Asset path was empty. Loading debug triangle.\n");

    const float X = 5.f;
    const float Y = 5.f;
    const float Z = 0.f;
    const float W = 1.f;
    const float v1[]{ -X, 0.f, Z, W, 0.f, 1.f, 0.f };
    const float v2[]{ 0.f, Y, Z, W, 1.0f, 0.f, 0.f };
    const float v3[]{ X, 0.f, Z, W, 0.0f, 0.f , 1.f };

    ShadingModeOrder order;
    ShadingMode mode(ShadingModes::RGB, 3);
    order.PushBack(mode);

    mWorld.DebugLoadTriangle(v1, v2, v3, order, 7);
  }
#endif
}

void GameInstance::MoveCamera(Direction direction) {
  Vec3 cameraLook(mCamera.GetLook());

  switch (direction) {
  case Direction::FORWARD:
    mCamera.Position() = mCamera.Position() + cameraLook;
    break;
  case Direction::BACK:
    mCamera.Position() = mCamera.Position() - cameraLook;
    break;
  case Direction::LEFT:
  {
    Vec3 sideVec(mCamera.GetUp().Cross(cameraLook));
    mCamera.Position() = mCamera.Position() - sideVec;
  }
  break;
  case Direction::RIGHT:
  {
    Vec3 sideVec(mCamera.GetUp().Cross(cameraLook));
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
  LoadAssets();

  mCamera.Position() = Vec3(0.f, 0.f, -40.f);

  InputManager& inputManager = InputManager::GetInstance();
  inputManager.OnKeyDownDelegate.Add(Delegate<uint8>::FromMember<GameInstance, &GameInstance::OnKeyDown>(this));
  inputManager.OnKeyUpDelegate.Add(Delegate<uint8>::FromMember<GameInstance, &GameInstance::OnKeyUp>(this));
  inputManager.OnMouseMoveDelegate.Add(Delegate<int32, int32, int32, int32>::FromMember<GameInstance, &GameInstance::OnMouseMove>(this));
}

void GameInstance::Tick() {
  NamedScopedTimer(Render);

  size_t frameDelta;

  if (mLastFrameTime == 0) {
    frameDelta = static_cast<size_t>(FRAMERATE_60HZ_MS);
  }
  else {
    frameDelta = PlatformHighResClockDelta(mLastFrameTime, ClockUnits::Milliseconds);
  }

  mLastFrameTime = PlatformHighResClock();

  {
    String frame;
    frame.Appendf("Frame: {0}\n", mFrameCount);
    Logger::Log(LogCategory::Info, frame);
  }

  {
    String delta;
    delta.Appendf("Frame Delta (ms): {0}\n", frameDelta);
    Logger::Log(LogCategory::Info, delta);
  }

  {
    String cameraPosition(mCamera.Position().ToString());
    cameraPosition.Append("\n");
    Logger::Log(LogCategory::Info, cameraPosition);
  }

  ++mFrameCount;

  InputManager& inputManager = InputManager::GetInstance();
  inputManager.Process();

#if !DISABLE_DEBUG_TRANSFORMS
  Vec3 rotation;
  rotation[1] = DegreesToRadians(static_cast<float>(mRotationAmount % 360));

  if (!mPauseTransforms) {
    mRotationAmount += mRotationSpeed;
  }

  for (Model& model : mWorld.GetModels()) {
    model.Rotation() = rotation;
  }
#endif

  mRenderer.RenderNextFrame(mWorld, mCamera);
}

void GameInstance::TickAudio() {
  size_t audioDelta = 1;

  if (mLastAudioTime > 0) {
    audioDelta = PlatformHighResClockDelta(mLastAudioTime, ClockUnits::Milliseconds);
  }

  mLastAudioTime = PlatformHighResClock();

  if (mAudioDevice != nullptr && mAmbientTrack.data != nullptr) {
    // TODO: This forces the track to loop.
    if (mAudioPosition >= mAmbientTrack.length) {
      mAudioPosition = 0;
    }

    // TODO: Audio is still a little choppy, fix this.
    mAudioPosition += PlatformPlayAudio(mAudioDevice, mAmbientTrack.data, mAudioPosition, mAmbientTrack.length);
  }
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
    mRenderer.ToggleRenderMode(RenderMode::WIREFRAME);
    break;
  case 'F':
    mRenderer.ToggleRenderMode(RenderMode::FLAT);
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
  int32 width = (int32)config.GetViewportWidth().Value();
  int32 height = (int32)config.GetViewportHeight().Value();

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

  return Vec3(newX, newY, -z);
}

}
