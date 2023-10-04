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

#include "PNG.h"
#include "PlatformFile.h"
#include "PlatformMemory.h"
#include "PlatformAudio.h"
#include "DebugText.h"

#include "Bundle.h"

#include "MP3.h"

#include <cmath>

#define DEBUG_AUDIO 0
#define DEBUG_TRIANGLE 0
#define DEBUG_TRIANGLE_TEXTURE 0
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

  mAmbientTrack = audioFile.DecompressFloat();
  if (mAmbientTrack.data != nullptr) {
    mAudioDevice = PlatformInitializeAudioDevice(mAmbientTrack.samplesPerSecond, mAmbientTrack.channels, 10);
    ZAssert(mAudioDevice != nullptr);
  }
#endif

#if DEBUG_TRIANGLE
  const float X = 5.f;
  const float Y = 5.f;
  const float Z = 0.f;
  const float W = 1.f;
  const float v1[]{ -X, 0.f, Z, W, 1.f, 0.f, 0.f };
  const float v2[]{ 0.f, Y, Z, W, 0.0f, 1.f, 0.f };
  const float v3[]{ X, 0.f, Z, W, 0.0f, 0.f, 1.f };

  ShadingModeOrder order;
  ShadingMode mode(ShadingModes::RGB, 3);
  order.PushBack(mode);

  mWorld.DebugLoadTriangle(v3, v2, v1, order, 7);
#elif DEBUG_TRIANGLE_TEXTURE
  const float X = 5.f;
  const float Y = 5.f;
  const float Z = 0.f;
  const float W = 1.f;
  const float v1[]{ -X, 0.f, Z, W, 1.f, 1.f };
  const float v2[]{ 0.f, Y, Z, W, 0.f, 1.f };
  const float v3[]{ X, 0.f, Z, W, 0.5f, 0.f };

  ShadingModeOrder order;
  ShadingMode mode(ShadingModes::UV, 2);
  order.PushBack(mode);

  mWorld.DebugLoadTriangle(v3, v2, v1, order, 6);
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

    mWorld.DebugLoadTriangle(v3, v2, v1, order, 7);
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
    mCamera.Position() = mCamera.Position() + sideVec;
  }
  break;
  case Direction::RIGHT:
  {
    Vec3 sideVec(mCamera.GetUp().Cross(cameraLook));
    mCamera.Position() = mCamera.Position() - sideVec;
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

  mCamera.Position() = Vec3(0.f, 0.f, 40.f);
  // Clip the model at the origin by moving the camera far away.
  // From there we can see how long the clipping pass takes for a given scene.
  //mCamera.Position() = Vec3(0.f, 0.f, 200.f);

  InputManager& inputManager = InputManager::GetInstance();
  inputManager.OnKeyDownDelegate.Add(Delegate<uint8>::FromMember<GameInstance, &GameInstance::OnKeyDown>(this));
  inputManager.OnKeyUpDelegate.Add(Delegate<uint8>::FromMember<GameInstance, &GameInstance::OnKeyUp>(this));
  inputManager.OnMouseMoveDelegate.Add(Delegate<int32, int32, int32, int32>::FromMember<GameInstance, &GameInstance::OnMouseMove>(this));
}

void GameInstance::Tick() {
  NamedScopedTimer(Render);

  size_t frameDelta = (mLastFrameTime == 0) ? static_cast<size_t>(FRAMERATE_60HZ_MS) : PlatformHighResClockDelta(mLastFrameTime, ClockUnits::Milliseconds);

  mLastFrameTime = PlatformHighResClock();

  const String frameString(String::FromFormat("Frame: {0}\n", mFrameCount));
  Logger::Log(LogCategory::Info, frameString);

  const String deltaString(String::FromFormat("Frame Delta (ms): {0}\n", frameDelta));
  Logger::Log(LogCategory::Info, deltaString);

  const String cameraPosition(String::FromFormat("Camera: {0}\n", mCamera.Position().ToString()));
  Logger::Log(LogCategory::Info, cameraPosition);

  const String cameraView(String::FromFormat("Camera View: {0}\n", mCamera.GetLook().ToString()));
  Logger::Log(LogCategory::Info, cameraView);

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

  if (mDrawStats) {
    Framebuffer& frameBuffer = mRenderer.GetFrameBuffer();
    DrawText(frameString, 10, 10, frameBuffer, ZColors::BLACK);
    DrawText(deltaString, 10, 20, frameBuffer, ZColors::BLACK);
    DrawText(cameraPosition, 10, 30, frameBuffer, ZColors::BLACK);
    DrawText(cameraView, 10, 40, frameBuffer, ZColors::BLACK);
  }
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
    mAudioPosition += PlatformPlayAudio(mAudioDevice, mAmbientTrack.data, mAudioPosition, mAmbientTrack.length, audioDelta);
  }
}

uint8* GameInstance::GetCurrentFrame() {
  if (!mVisualizeDepth) {
    return mRenderer.GetFrame();
  }
  else {
    return mRenderer.GetDepth();
  }
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
    MoveCamera(GameInstance::Direction::FORWARD);
    break;
  case 'S':
    MoveCamera(GameInstance::Direction::BACK);
    break;
  case 'A':
    MoveCamera(GameInstance::Direction::LEFT);
    break;
  case 'D':
    MoveCamera(GameInstance::Direction::RIGHT);
    break;
  case 'Q':
    RotateCamera(Mat4x4::Axis::Y, 1.0F);
    break;
  case 'E':
    RotateCamera(Mat4x4::Axis::Y, -1.0F);
    break;
  case 'I':
    mDrawStats = !mDrawStats;
    break;
  case 'Z':
    mVisualizeDepth = !mVisualizeDepth;
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
  float theta = V1 * V2;
  Quaternion quat(theta, normal);
  RotateTrackball(quat);
}

Vec3 GameInstance::ProjectClick(float x, float y) {
  ZConfig& config = ZConfig::GetInstance();
  float width = (float)config.GetViewportWidth().Value();
  float height = (float)config.GetViewportHeight().Value();

  // TODO: The bigger the radius, the slower the rotation.
  // Maybe we could use this to adjust the speed at which the camera spins?
  float radius = width * 5.f;

  float newX = ((x - (width / 2.f)) / radius);
  float newY = ((y - (height / 2.f)) / radius);

  // Note that we flip the X axis so that left/right rotation looks natural.

  float r = (newX * newX) + (newY * newY);
  if (r > 1.f) {
    float s = 1.f / sqrtf(r);
    return Vec3(-newX * s, newY * s, 0.f);
  }
  else {
    return Vec3(-newX, newY, sqrtf(1.f - r));
  }
}

}
