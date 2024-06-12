#include "GameInstance.h"

#include "Bundle.h"
#include "CommonMath.h"
#include "Constants.h"
#include "ConsoleVariable.h"
#include "DebugText.h"
#include "Logger.h"
#include "PlatformTime.h"
#include "PlatformMemory.h"
#include "PlatformAudio.h"
#include "ScopedTimer.h"
#include "ShadingMode.h"
#include "ZConfig.h"
#include "ZString.h"

#include "PlatformIntrinsics.h"

#include <cmath>

namespace ZSharp {
ConsoleVariable<bool> DebugTransforms("DebugTransforms", true);
ConsoleVariable<float> CameraSpeed("CameraSpeed", 1.f);

GameInstance::GameInstance() {

}

GameInstance::~GameInstance() {
  InputManager& inputManager = InputManager::Get();
  inputManager.OnKeyDownDelegate.Remove(Delegate<uint8>::FromMember<GameInstance, &GameInstance::OnKeyDown>(this));
  inputManager.OnKeyUpDelegate.Remove(Delegate<uint8>::FromMember<GameInstance, &GameInstance::OnKeyUp>(this));
  inputManager.OnMiscKeyDownDelegate.Remove(Delegate<MiscKey>::FromMember<GameInstance, &GameInstance::OnMiscKeyDown>(this));
  inputManager.OnMiscKeyUpDelegate.Remove(Delegate<MiscKey>::FromMember<GameInstance, &GameInstance::OnMiscKeyUp>(this));
  inputManager.OnMouseDragDelegate.Remove(Delegate<int32, int32, int32, int32>::FromMember<GameInstance, &GameInstance::OnMouseMove>(this));
}

void GameInstance::LoadWorld() {
  NamedScopedTimer(WorldLoad);

  mWorld.Load();

  mCamera.Position() = Vec3(0.f, 5.f, 50.f);
  // Clip the model at the origin by moving the camera far away.
  // From there we can see how long the clipping pass takes for a given scene.
  //mCamera.Position() = Vec3(0.f, 0.f, 200.f);

  InputManager& inputManager = InputManager::Get();
  inputManager.OnKeyDownDelegate.Add(Delegate<uint8>::FromMember<GameInstance, &GameInstance::OnKeyDown>(this));
  inputManager.OnKeyUpDelegate.Add(Delegate<uint8>::FromMember<GameInstance, &GameInstance::OnKeyUp>(this));
  inputManager.OnMiscKeyDownDelegate.Add(Delegate<MiscKey>::FromMember<GameInstance, &GameInstance::OnMiscKeyDown>(this));
  inputManager.OnMiscKeyUpDelegate.Add(Delegate<MiscKey>::FromMember<GameInstance, &GameInstance::OnMiscKeyUp>(this));
  inputManager.OnMouseDragDelegate.Add(Delegate<int32, int32, int32, int32>::FromMember<GameInstance, &GameInstance::OnMouseMove>(this));
}

void GameInstance::LoadFrontEnd() {
  NamedScopedTimer(FrontEndLoad);

  mFrontEnd.Load();
}

void GameInstance::TickWorld() {
  NamedScopedTimer(TickWorld);

  size_t renderFrameTime = PlatformHighResClock();

  {
    NamedScopedTimer(ThreadPoolWait);
    mThreadPool.WaitForJobs();
  }

  size_t frameDeltaMs = (mLastFrameTime == 0) ? FRAMERATE_60HZ_MS : PlatformHighResClockDelta(mLastFrameTime, ClockUnits::Milliseconds);
  mLastFrameTime = PlatformHighResClock();

  const String frameString(String::FromFormat("Frame: {0}\n", mFrameCount));
  Logger::Log(LogCategory::Info, frameString);

  const String deltaString(String::FromFormat("Frame Delta: {0}ms\n", frameDeltaMs));
  Logger::Log(LogCategory::Info, deltaString);

  const String cameraPosition(String::FromFormat("Camera: {0}\n", mCamera.Position().ToString()));
  Logger::Log(LogCategory::Info, cameraPosition);

  const String cameraView(String::FromFormat("Camera View: {0}\n", mCamera.GetLook().ToString()));
  Logger::Log(LogCategory::Info, cameraView);

  size_t numModels = mWorld.GetModels().Size();
  size_t numVerts = 0;
  size_t numTriangles = 0;

  for (Model& model : mWorld.GetModels()) {
    for (Mesh& mesh : model.GetMeshData()) {
      numVerts += (mesh.GetVertTable().Size() / mesh.Stride());
      numTriangles += mesh.GetTriangleFaceTable().Size();
    }
  }

  const String modelString(String::FromFormat("Num Models: {0}\n", numModels));
  Logger::Log(LogCategory::Info, modelString);

  const String vertString(String::FromFormat("Num Verts: {0}\n", numVerts));
  Logger::Log(LogCategory::Info, vertString);

  const String triangleString(String::FromFormat("Num Triangles: {0}\n", numTriangles));
  Logger::Log(LogCategory::Info, triangleString);

  ++mFrameCount;

  InputManager& inputManager = InputManager::Get();
  inputManager.Process();

  if (*DebugTransforms) {
    Vec3 rotation;
    rotation[1] = DegreesToRadians(static_cast<float>(mRotationAmount % 360));

    if (!mPauseTransforms) {
      mRotationAmount += mRotationSpeed;
    }

    for (Model& model : mWorld.GetModels()) {
      // TODO: Hacking some stuff together real quick for physics.
      if (model.Tag() == PhysicsTag::Dynamic) {
        model.Rotation() = rotation;
      }
    }
  }

  mCamera.Tick();

  size_t physicsTickTime = Clamp(frameDeltaMs, (size_t)0, FRAMERATE_60HZ_MS);

  size_t startPhysics = PlatformHighResClockDelta(mLastFrameTime, ClockUnits::Microseconds);
  mWorld.TickPhysics(physicsTickTime);
  size_t endPhysics = PlatformHighResClockDelta(mLastFrameTime, ClockUnits::Microseconds);
  const String physicsTime(String::FromFormat("Physics time: {0}us\n", endPhysics - startPhysics));

  mRenderer.RenderNextFrame(mWorld, mCamera);

  size_t remainingTriangles = 0;
  for (IndexBuffer& indexBuffer : mWorld.GetIndexBuffers()) {
    if (indexBuffer.WasClipped()) {
      remainingTriangles += indexBuffer.GetClipLength() / 3;
    }
    else {
      remainingTriangles += indexBuffer.GetIndexSize() / 3;
    }
  }

  float cullRatio = (float)remainingTriangles / (float)numTriangles;
  const String remainingTriangleString(String::FromFormat("Post Clip/Cull Triangles: {0}, {1}%\n", remainingTriangles, cullRatio));
  Logger::Log(LogCategory::Info, remainingTriangleString);

  if (mDrawStats) {
    String renderFrameTimeString(String::FromFormat("Render Frame: {0}us", PlatformHighResClockDelta(renderFrameTime, ClockUnits::Microseconds)));

    size_t bufferWidth = mRenderer.GetFrameBuffer().GetWidth();
    uint8* buffer;
    ZColor color;
    if (mVisualizeDepth) {
      buffer = mRenderer.GetDepth();
      color = ZColors::WHITE;
    }
    else {
      buffer = mRenderer.GetFrameBuffer().GetBuffer();
      color = ZColors::BLACK;
    }

    // TODO: Organize stats a bit better.
    DrawText(frameString, 10, 10, buffer, bufferWidth, color);
    DrawText(deltaString, 10, 20, buffer, bufferWidth, color);
    DrawText(renderFrameTimeString, 10, 30, buffer, bufferWidth, color);
    DrawText(cameraPosition, 10, 40, buffer, bufferWidth, color);
    DrawText(cameraView, 10, 50, buffer, bufferWidth, color);
    DrawText(physicsTime, 10, 60, buffer, bufferWidth, color);
    DrawText(modelString, 10, 70, buffer, bufferWidth, color);
    DrawText(vertString, 10, 80, buffer, bufferWidth, color);
    DrawText(triangleString, 10, 90, buffer, bufferWidth, color);
    DrawText(remainingTriangleString, 10, 100, buffer, bufferWidth, color);
  }

  if (mDevConsole.IsOpen()) {
    uint8* buffer = mVisualizeDepth ? mRenderer.GetDepth() : mRenderer.GetFrameBuffer().GetBuffer();
    mDevConsole.Draw((uint32*)buffer);
  }
}

void GameInstance::TickFrontEnd() {
  NamedScopedTimer(TickFrontEnd);

  mFrontEnd.Tick();

  InputManager& inputManager = InputManager::Get();
  inputManager.Process();

  ZColor clearColor(ZColors::BLACK);
  mRenderer.ClearFramebuffer(clearColor);

  Framebuffer& framebuffer = mRenderer.GetFrameBuffer();
  uint8* buffer = mRenderer.GetFrame();
  size_t width = framebuffer.GetWidth();
  size_t height = framebuffer.GetHeight();

  if (!mFrontEnd.IsVisible()) {
    ZColor loadColor(ZColors::WHITE);
    DrawText("LOADING WORLD", width / 2, height / 2, buffer, width, loadColor);
    return;
  }

  mFrontEnd.Draw(buffer, width, height);
}

void GameInstance::MoveCamera(Direction direction) {
  Vec3 cameraLook(mCamera.GetLook());

  cameraLook *= (*CameraSpeed);

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

void GameInstance::RotateTrackball(const Quaternion& quat) {
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

void GameInstance::Initialize(bool skipTitleScreen) {
  if (!skipTitleScreen) {
    LoadWorld();
  }
  else {
    LoadFrontEnd();
  }
}

void GameInstance::Tick() {
  if (mFrontEnd.IsVisible()) {
    TickFrontEnd();
  }
  else {
    if (!mWorld.IsLoaded()) {
      LoadWorld();
    }

    TickWorld();
  }
}

void GameInstance::TickAudio() {
  size_t audioDelta = 1;

  if (mLastAudioTime > 0) {
    audioDelta = PlatformHighResClockDelta(mLastAudioTime, ClockUnits::Milliseconds);
  }

  mLastAudioTime = PlatformHighResClock();

  mWorld.TickAudio(audioDelta);
}

uint8* GameInstance::GetCurrentFrame() {
  if (mFrontEnd.IsVisible()) {
    return mRenderer.GetFrame();
  }
  else if (!mVisualizeDepth) {
    return mRenderer.GetFrame();
  }
  else {
    return mRenderer.GetDepth();
  }
}

bool GameInstance::IsDevConsoleOpen() const {
  return mDevConsole.IsOpen();
}

void GameInstance::RunBackgroundJobs() {
  if (!mWorld.IsLoaded()) {
    return;
  }

  mThreadPool.Wake();

  ParallelRange range = ParallelRange::FromMember<GameInstance, &GameInstance::FastClearBuffers>(this);
  size_t size = mRenderer.GetFrameBuffer().GetSize();

  mThreadPool.Execute(range, nullptr, size, 4, true);
}

void GameInstance::WaitForBackgroundJobs() {
  NamedScopedTimer(ThreadPoolWait);
  mThreadPool.WaitForJobs();
}

void GameInstance::OnKeyDown(uint8 key) {
  if (mDevConsole.IsOpen()) {
    return;
  }

  switch (key) {
  case 'p':
    PauseTransforms();
    break;
  case 'r':
    mRenderer.ToggleRenderMode(RenderMode::WIREFRAME);
    break;
  case 'f':
    mRenderer.ToggleRenderMode(RenderMode::FLAT);
    break;
  case 'w':
    MoveCamera(GameInstance::Direction::FORWARD);
    break;
  case 's':
    MoveCamera(GameInstance::Direction::BACK);
    break;
  case 'a':
    MoveCamera(GameInstance::Direction::LEFT);
    break;
  case 'd':
    MoveCamera(GameInstance::Direction::RIGHT);
    break;
  case 'q':
    RotateCamera(Mat4x4::Axis::Y, 1.0F);
    break;
  case 'e':
    RotateCamera(Mat4x4::Axis::Y, -1.0F);
    break;
  case 'i':
    mDrawStats = !mDrawStats;
    break;
  case 'z':
    mVisualizeDepth = !mVisualizeDepth;
    break;
  default:
    break;
  }
}

void GameInstance::OnKeyUp(uint8 key) {
  (void)key;
}

void GameInstance::OnMiscKeyDown(MiscKey key) {
  if (mDevConsole.IsOpen()) {
    return;
  }

  switch (key) {
    case MiscKey::UP_ARROW:
      ChangeSpeed(1);
      break;
    case MiscKey::DOWN_ARROW:
      ChangeSpeed(-1);
      break;
    default:
      break;
  }
}

void GameInstance::OnMiscKeyUp(MiscKey key) {
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

void GameInstance::FastClearBuffers(size_t begin, size_t end, void* data) {
  (void)data;

  ZColor orange(ZColors::ORANGE);
  mRenderer.GetFrameBuffer().Clear(orange, begin, end - begin);
  mRenderer.GetDepthBuffer().Clear(begin, end - begin);
}

Vec3 GameInstance::ProjectClick(float x, float y) {
  ZConfig& config = ZConfig::Get();
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
