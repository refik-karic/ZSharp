#include "GameInstance.h"

#include "Bundle.h"
#include "CommonMath.h"
#include "Constants.h"
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
ConsoleVariable<ZColor> ClearColor("ClearColor", ZColor(ZColors::ORANGE));

GameInstance::GameInstance()
  : 
    mFrontEnd(new FrontEnd()), mWorld(new World()), mRenderer(new Renderer()), 
    mThreadPool(new ThreadPool()), mExtraState(new ExtraState()), mDevConsole(new DevConsole()),
    mCameraReset(new ConsoleVariable<void>("CameraReset", Delegate<void>::FromMember<GameInstance, &GameInstance::ResetCamera>(this))) {
  mExtraState->mPauseTransforms = false;
  mExtraState->mDrawStats = true;
  mExtraState->mVisualizeDepth = false;
  mPlayer = new Player(mDevConsole);
}

GameInstance::~GameInstance() {
  if (mCameraReset) {
    delete mCameraReset;
  }

  if (mFrontEnd) {
    delete mFrontEnd;
  }

  if (mPlayer) {
    delete mPlayer;
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
  inputManager->OnMiscKeyDownDelegate.Remove(Delegate<MiscKey>::FromMember<GameInstance, &GameInstance::OnMiscKeyDown>(this));
}

void GameInstance::LoadWorld() {
  NamedScopedTimer(WorldLoad);

  mWorld->AssignPlayer(mPlayer);
  mPlayer->Load();

  mWorld->Load();

  // Clip the model at the origin by moving the camera far away.
  // From there we can see how long the clipping pass takes for a given scene.
  //mCamera.Position() = {0.f, 0.f, 200.f};

  InputManager* inputManager = GlobalInputManager;
  inputManager->OnKeyDownDelegate.Add(Delegate<uint8>::FromMember<GameInstance, &GameInstance::OnKeyDown>(this));
  inputManager->OnMiscKeyDownDelegate.Add(Delegate<MiscKey>::FromMember<GameInstance, &GameInstance::OnMiscKeyDown>(this));
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
  Logger::Log(LogCategory::Info, stats.EmplaceBack(String::FromFormat("Camera: {0}\n", mPlayer->Position().ToString())));
  Logger::Log(LogCategory::Info, stats.EmplaceBack(String::FromFormat("Camera View: {0}\n", mPlayer->ViewCamera()->GetLook().ToString())));

  size_t numModels = mWorld->GetModels().Size();
  size_t numVerts = 0;
  size_t numTriangles = 0;

  for (Model& model : mWorld->GetModels()) {
    Mesh& mesh = model.GetMesh();
    numVerts += (mesh.GetVertTable().Size() / mesh.Stride());
    numTriangles += mesh.GetTriangleFaceTable().Size();
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
        model.Rotation() = Quaternion(DegreesToRadians(static_cast<float>(mExtraState->mRotationAmount % 360)), { 0.f, 1.f, 0.f });
      }
    }
  }

  size_t physicsTickTime = Clamp(frameDeltaMs, (size_t)0, FRAMERATE_60HZ_MS);

  size_t startPhysics = PlatformHighResClockDeltaUs(mExtraState->mLastFrameTime);
  mWorld->TickPhysics(physicsTickTime);
  size_t endPhysics = PlatformHighResClockDeltaUs(mExtraState->mLastFrameTime);

  Logger::Log(LogCategory::Info, stats.EmplaceBack(String::FromFormat("Physics time: {0}us\n", endPhysics - startPhysics)));

  mPlayer->Tick();

  mRenderer->RenderNextFrame(*mWorld, *mPlayer->ViewCamera());

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
    mPlayer->ResetCamera();
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

void InitializeGlobals() {
  GlobalLog = new Logger();

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

  if (GlobalLog) {
    delete GlobalLog;
  }
}

}
