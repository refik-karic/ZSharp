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
#include "TexturePool.h"
#include "ZConfig.h"
#include "ZString.h"

#include "PlatformIntrinsics.h"

#include <cmath>

namespace ZSharp {
ConsoleVariable<bool> DebugTransforms("DebugTransforms", true);
ConsoleVariable<float> CameraSpeed("CameraSpeed", 1.f);
ConsoleVariable<float> CameraRotation("CameraRotation", 5.f);
ConsoleVariable<ZColor> ClearColor("ClearColor", ZColor(ZColors::ORANGE));

GameInstance::GameInstance()
  : 
    mFrontEnd(new FrontEnd()), mCamera(new Camera()), mWorld(new World()), mRenderer(new Renderer()), 
    mThreadPool(new ThreadPool()), mExtraState(new ExtraState()), mDevConsole(new DevConsole()),
    mCameraReset(new ConsoleVariable<void>("CameraReset", Delegate<void>::FromMember<GameInstance, &GameInstance::ResetCamera>(this))) {

}

GameInstance::~GameInstance() {
  if (mCameraReset) {
    delete mCameraReset;
  }

  if (mFrontEnd) {
    delete mFrontEnd;
  }

  if (mCamera) {
    delete mCamera;
  }

  if (mWorld) {
    delete mWorld;
  }

  if (mRenderer) {
    delete mRenderer;
  }

  if (mDevConsole) {
    delete mDevConsole;
  }

  if (mThreadPool) {
    delete mThreadPool;
  }

  if (mExtraState) {
    delete mExtraState;
  }

  InputManager* inputManager = GlobalInputManager;
  inputManager->OnKeyDownDelegate.Remove(Delegate<uint8>::FromMember<GameInstance, &GameInstance::OnKeyDown>(this));
  inputManager->OnKeyUpDelegate.Remove(Delegate<uint8>::FromMember<GameInstance, &GameInstance::OnKeyUp>(this));
  inputManager->OnMiscKeyDownDelegate.Remove(Delegate<MiscKey>::FromMember<GameInstance, &GameInstance::OnMiscKeyDown>(this));
  inputManager->OnMiscKeyUpDelegate.Remove(Delegate<MiscKey>::FromMember<GameInstance, &GameInstance::OnMiscKeyUp>(this));
  inputManager->OnMouseDragDelegate.Remove(Delegate<int32, int32, int32, int32>::FromMember<GameInstance, &GameInstance::OnMouseMove>(this));
}

void GameInstance::LoadWorld() {
  NamedScopedTimer(WorldLoad);

  mWorld->Load();

  mCamera->Position() = Vec3(0.f, 5.f, 50.f);
  // Clip the model at the origin by moving the camera far away.
  // From there we can see how long the clipping pass takes for a given scene.
  //mCamera.Position() = Vec3(0.f, 0.f, 200.f);

  InputManager* inputManager = GlobalInputManager;
  inputManager->OnKeyDownDelegate.Add(Delegate<uint8>::FromMember<GameInstance, &GameInstance::OnKeyDown>(this));
  inputManager->OnKeyUpDelegate.Add(Delegate<uint8>::FromMember<GameInstance, &GameInstance::OnKeyUp>(this));
  inputManager->OnMiscKeyDownDelegate.Add(Delegate<MiscKey>::FromMember<GameInstance, &GameInstance::OnMiscKeyDown>(this));
  inputManager->OnMiscKeyUpDelegate.Add(Delegate<MiscKey>::FromMember<GameInstance, &GameInstance::OnMiscKeyUp>(this));
  inputManager->OnMouseDragDelegate.Add(Delegate<int32, int32, int32, int32>::FromMember<GameInstance, &GameInstance::OnMouseMove>(this));
}

void GameInstance::LoadFrontEnd() {
  NamedScopedTimer(FrontEndLoad);

  mFrontEnd->Load();
}

void GameInstance::TickWorld() {
  NamedScopedTimer(TickWorld);

  size_t renderFrameTime = PlatformHighResClock();

  {
    NamedScopedTimer(ThreadPoolWait);
    mThreadPool->WaitForJobs();
  }

  Array<String> stats;

  size_t frameDeltaMs = (mExtraState->mLastFrameTime == 0) ? FRAMERATE_60HZ_MS : PlatformHighResClockDeltaMs(mExtraState->mLastFrameTime);
  mExtraState->mLastFrameTime = PlatformHighResClock();

  Logger::Log(LogCategory::Info, stats.EmplaceBack(String::FromFormat("Frame: {0}\n", mExtraState->mFrameCount)));
  Logger::Log(LogCategory::Info, stats.EmplaceBack(String::FromFormat("Frame Delta: {0}ms\n", frameDeltaMs)));
  Logger::Log(LogCategory::Info, stats.EmplaceBack(String::FromFormat("Camera: {0}\n", mCamera->Position().ToString())));
  Logger::Log(LogCategory::Info, stats.EmplaceBack(String::FromFormat("Camera View: {0}\n", mCamera->GetLook().ToString())));

  size_t numModels = mWorld->GetModels().Size();
  size_t numVerts = 0;
  size_t numTriangles = 0;

  for (Model& model : mWorld->GetModels()) {
    for (Mesh& mesh : model.GetMeshData()) {
      numVerts += (mesh.GetVertTable().Size() / mesh.Stride());
      numTriangles += mesh.GetTriangleFaceTable().Size();
    }
  }

  Logger::Log(LogCategory::Info, stats.EmplaceBack(String::FromFormat("Num Models: {0}\n", numModels)));
  Logger::Log(LogCategory::Info, stats.EmplaceBack(String::FromFormat("Num Verts: {0}\n", numVerts)));
  Logger::Log(LogCategory::Info, stats.EmplaceBack(String::FromFormat("Num Triangles: {0}\n", numTriangles)));

  ++(mExtraState->mFrameCount);

  InputManager* inputManager = GlobalInputManager;
  inputManager->Process();

  if (*DebugTransforms) {
    if (!(mExtraState->mPauseTransforms)) {
      mExtraState->mRotationAmount += mExtraState->mRotationSpeed;
    }

    for (Model& model : mWorld->GetModels()) {
      // TODO: Hacking some stuff together real quick for physics.
      if (model.Tag() == PhysicsTag::Dynamic) {
        model.Rotation() = Quaternion(DegreesToRadians(static_cast<float>(mExtraState->mRotationAmount % 360)), Vec3(0.f, 1.f, 0.f));
      }
    }
  }

  mCamera->Tick();

  size_t physicsTickTime = Clamp(frameDeltaMs, (size_t)0, FRAMERATE_60HZ_MS);

  size_t startPhysics = PlatformHighResClockDeltaUs(mExtraState->mLastFrameTime);
  mWorld->TickPhysics(physicsTickTime);
  size_t endPhysics = PlatformHighResClockDeltaUs(mExtraState->mLastFrameTime);

  Logger::Log(LogCategory::Info, stats.EmplaceBack(String::FromFormat("Physics time: {0}us\n", endPhysics - startPhysics)));

  mRenderer->RenderNextFrame(*mWorld, *mCamera);

  size_t remainingTriangles = 0;
  for (IndexBuffer& indexBuffer : mWorld->GetIndexBuffers()) {
    if (indexBuffer.WasClipped()) {
      remainingTriangles += indexBuffer.GetClipLength() / 3;
    }
    else {
      remainingTriangles += indexBuffer.GetIndexSize() / 3;
    }
  }

  float cullRatio = (float)remainingTriangles / (float)numTriangles;
  Logger::Log(LogCategory::Info, stats.EmplaceBack(String::FromFormat("Post Clip/Cull Triangles: {0}, {1:4}%\n", remainingTriangles, cullRatio)));

  if (mExtraState->mDrawStats) {
    stats.EmplaceBack(String::FromFormat("Render Frame: {0}us", PlatformHighResClockDeltaUs(renderFrameTime)));

    size_t bufferWidth = mRenderer->GetFrameBuffer().GetWidth();
    uint8* buffer;
    ZColor color;
    if (mExtraState->mVisualizeDepth) {
      buffer = mRenderer->GetDepth();
      color = ZColors::GREEN;
    }
    else {
      buffer = mRenderer->GetFrameBuffer().GetBuffer();
      color = ZColors::BLACK;
    }

    size_t y = 10;
    for (size_t i = 0; i < stats.Size(); ++i) {
      DrawText(stats[i], 10, y, buffer, bufferWidth, color);
      y += 10;
    }
  }

  if (mDevConsole->IsOpen()) {
    uint8* buffer = mExtraState->mVisualizeDepth ? mRenderer->GetDepth() : mRenderer->GetFrameBuffer().GetBuffer();
    mDevConsole->Draw((uint32*)buffer);
  }
}

void GameInstance::TickFrontEnd() {
  NamedScopedTimer(TickFrontEnd);

  mFrontEnd->Tick();

  InputManager* inputManager = GlobalInputManager;
  inputManager->Process();

  ZColor clearColor(ZColors::BLACK);
  mRenderer->ClearFramebuffer(clearColor);

  Framebuffer& framebuffer = mRenderer->GetFrameBuffer();
  uint8* buffer = mRenderer->GetFrame();
  size_t width = framebuffer.GetWidth();
  size_t height = framebuffer.GetHeight();

  if (!mFrontEnd->IsVisible()) {
    ZColor loadColor(ZColors::WHITE);
    DrawText("LOADING WORLD", width / 2, height / 2, buffer, width, loadColor);
    return;
  }

  mFrontEnd->Draw(buffer, width, height);
}

void GameInstance::MoveCamera(Direction direction) {
  Vec3 cameraLook(mCamera->GetLook());

  cameraLook *= (*CameraSpeed);

  switch (direction) {
  case Direction::FORWARD:
    mCamera->Position() = mCamera->Position() + cameraLook;
    break;
  case Direction::BACK:
    mCamera->Position() = mCamera->Position() - cameraLook;
    break;
  case Direction::LEFT:
  {
    Vec3 sideVec(mCamera->GetUp().Cross(cameraLook));
    mCamera->Position() = mCamera->Position() + sideVec;
  }
  break;
  case Direction::RIGHT:
  {
    Vec3 sideVec(mCamera->GetUp().Cross(cameraLook));
    mCamera->Position() = mCamera->Position() - sideVec;
  }
  break;
  }
}

void GameInstance::RotateCamera(const Quaternion& quat) {
  mCamera->RotateCamera(quat);
}

void GameInstance::ChangeSpeed(int64 amount) {
  if (mExtraState->mRotationSpeed + amount > 10) {
    mExtraState->mRotationSpeed = 10;
  }
  else if (mExtraState->mRotationSpeed + amount < 1) {
    mExtraState->mRotationSpeed = 1;
  }
  else {
    mExtraState->mRotationSpeed += amount;
  }
}

void GameInstance::ResetCamera() {
  if (!mFrontEnd->IsVisible() && mWorld->IsLoaded()) {
    mCamera->ResetOrientation();
    mCamera->Position() = Vec3(0.f, 5.f, 50.f);
  }
}

void GameInstance::PauseTransforms() {
  mExtraState->mPauseTransforms = !(mExtraState->mPauseTransforms);
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
  if (mFrontEnd->IsVisible()) {
    TickFrontEnd();
  }
  else {
    if (!mWorld->IsLoaded()) {
      LoadWorld();
    }

    TickWorld();
  }
}

void GameInstance::TickAudio() {
  size_t audioDelta = 1;

  if (mExtraState->mLastAudioTime > 0) {
    audioDelta = PlatformHighResClockDeltaMs(mExtraState->mLastAudioTime);
  }

  mExtraState->mLastAudioTime = PlatformHighResClock();

  mWorld->TickAudio(audioDelta);
}

uint8* GameInstance::GetCurrentFrame() {
  if (mFrontEnd->IsVisible()) {
    return mRenderer->GetFrame();
  }
  else if (!(mExtraState->mVisualizeDepth)) {
    return mRenderer->GetFrame();
  }
  else {
    return mRenderer->GetDepth();
  }
}

bool GameInstance::IsDevConsoleOpen() const {
  return mDevConsole->IsOpen();
}

void GameInstance::RunBackgroundJobs() {
  if (!mWorld->IsLoaded()) {
    return;
  }

  ParallelRange frameBufferClear = ParallelRange::FromMember<GameInstance, &GameInstance::FastClearFrameBuffer>(this);
  ParallelRange depthBufferClear = ParallelRange::FromMember<GameInstance, &GameInstance::FastClearDepthBuffer>(this);
  size_t size = mRenderer->GetFrameBuffer().GetWidth() * mRenderer->GetFrameBuffer().GetHeight();

  mThreadPool->Execute(frameBufferClear, mRenderer->GetFrameBuffer().GetBuffer(), size);
  mThreadPool->Execute(depthBufferClear, mRenderer->GetDepthBuffer().GetBuffer(), size);
}

void GameInstance::WaitForBackgroundJobs() {
  NamedScopedTimer(ThreadPoolWait);
  mThreadPool->WaitForJobs();
}

void GameInstance::OnKeyDown(uint8 key) {
  if (mDevConsole->IsOpen()) {
    return;
  }

  InputManager* input = GlobalInputManager;

  switch (key) {
  case 'p':
    PauseTransforms();
    break;
  case 'r':
    mRenderer->ToggleRenderMode(RenderMode::WIREFRAME);
    break;
  case 'f':
    mRenderer->ToggleRenderMode(RenderMode::FILL);
    break;
  case 'w':
  {
    if (input->GetKeyState('a') == InputManager::KeyState::Down) {
      MoveCamera(GameInstance::Direction::LEFT);
    }
    else if (input->GetKeyState('d') == InputManager::KeyState::Down) {
      MoveCamera(GameInstance::Direction::RIGHT);
    }

    MoveCamera(GameInstance::Direction::FORWARD);
  }
    break;
  case 's':
  {
    if (input->GetKeyState('a') == InputManager::KeyState::Down) {
      MoveCamera(GameInstance::Direction::LEFT);
    }
    else if (input->GetKeyState('d') == InputManager::KeyState::Down) {
      MoveCamera(GameInstance::Direction::RIGHT);
    }

    MoveCamera(GameInstance::Direction::BACK);
  }
    break;
  case 'a':
  {
    if (input->GetKeyState('w') == InputManager::KeyState::Down) {
      MoveCamera(GameInstance::Direction::FORWARD);
    }
    else if (input->GetKeyState('s') == InputManager::KeyState::Down) {
      MoveCamera(GameInstance::Direction::BACK);
    }

    MoveCamera(GameInstance::Direction::LEFT);
  }
    break;
  case 'd':
  {
    if (input->GetKeyState('w') == InputManager::KeyState::Down) {
      MoveCamera(GameInstance::Direction::FORWARD);
    }
    else if (input->GetKeyState('s') == InputManager::KeyState::Down) {
      MoveCamera(GameInstance::Direction::BACK);
    }

    MoveCamera(GameInstance::Direction::RIGHT);
  }
    break;
  case 'q':
    RotateCamera(Quaternion(DegreesToRadians(1.f), Vec3(0.f, 1.f, 0.f)));
    break;
  case 'e':
    RotateCamera(Quaternion(DegreesToRadians(-1.f), Vec3(0.f, 1.f, 0.f)));
    break;
  case 'i':
    mExtraState->mDrawStats = !(mExtraState->mDrawStats);
    break;
  case 'z':
    mExtraState->mVisualizeDepth = !(mExtraState->mVisualizeDepth);
    break;
  default:
    break;
  }
}

void GameInstance::OnKeyUp(uint8 key) {
  (void)key;
}

void GameInstance::OnMiscKeyDown(MiscKey key) {
  if (mDevConsole->IsOpen()) {
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
  const Vec3 V1(ProjectClick((float)oldX, (float)oldY));
  const Vec3 V2(ProjectClick((float)x, (float)y));

  const Vec3 normal(V1.Cross(V2));
  float theta = V1 * V2;
  const Quaternion quat(theta, normal);
  RotateCamera(quat);
}

void GameInstance::FastClearFrameBuffer(Span<uint8> data) {
  Framebuffer& frameBuffer = mRenderer->GetFrameBuffer();
  const size_t chunkSize = 4;
  size_t start = data.GetData() - frameBuffer.GetBuffer();
  size_t length = data.Size();
  frameBuffer.Clear(*ClearColor, start * chunkSize, length * chunkSize);
}

void GameInstance::FastClearDepthBuffer(Span<uint8> data) {
  DepthBuffer& depthBuffer = mRenderer->GetDepthBuffer();
  size_t start = data.GetData() - ((uint8*)depthBuffer.GetBuffer());
  size_t length = data.Size();
  const size_t chunkSize = 4;
  depthBuffer.Clear(start * chunkSize, length * chunkSize);
}

Vec3 GameInstance::ProjectClick(float x, float y) {
  const ZConfig* config = GlobalConfig;
  float width = (float)config->GetViewportWidth().Value();
  float height = (float)config->GetViewportHeight().Value();

  float radius = width * (*CameraRotation);

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

void InitializeGlobals() {
  GlobalConfig = new ZConfig();

  GlobalBundle = new Bundle(GlobalConfig->GetAssetPath());

  GlobalInputManager = new InputManager();

  GlobalTexturePool = new TexturePool();
  
  // Ignoring AVX512 for now.
  if (PlatformSupportsSIMDLanes(SIMDLaneWidth::Eight)) {
    RGBShaderImpl = &Unaligned_Shader_RGB_AVX;
    UVShaderImpl = &Unaligned_Shader_UV_AVX;
    CalculateAABBImpl = &Unaligned_AABB_AVX;
    DrawDebugTextImpl = &Unaligned_DrawDebugText_AVX;
    DepthBufferVisualizeImpl = &Aligned_DepthBufferVisualize_AVX;
    BlendBuffersImpl = &Unaligned_BlendBuffers_AVX;
    BilinearScaleImageImpl = &Unaligned_BilinearScaleImage_AVX;
    GenerateMipLevelImpl = &Unaligned_GenerateMipLevel_AVX;
  }
  else if (PlatformSupportsSIMDLanes(SIMDLaneWidth::Four)) {
    RGBShaderImpl = &Unaligned_Shader_RGB_SSE;
    UVShaderImpl = &Unaligned_Shader_UV_SSE;
    CalculateAABBImpl = &Unaligned_AABB_SSE;
    DrawDebugTextImpl = &Unaligned_DrawDebugText_SSE;
    DepthBufferVisualizeImpl = &Aligned_DepthBufferVisualize_SSE;
    BlendBuffersImpl = &Unaligned_BlendBuffers_SSE;
    BilinearScaleImageImpl = &Unaligned_BilinearScaleImage_SSE;
    GenerateMipLevelImpl = &Unaligned_GenerateMipLevel_SSE;
  }
  else {
    ZAssert(false);
  }
}

void FreeGlobals() {
  if (GlobalTexturePool) {
    delete GlobalTexturePool;
  }

  if (GlobalInputManager) {
    delete GlobalInputManager;
  }

  if (GlobalBundle) {
    delete GlobalBundle;
  }

  if (GlobalConfig) {
    delete GlobalConfig;
  }
}

}
