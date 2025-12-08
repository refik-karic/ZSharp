#pragma once

#include "Camera.h"
#include "FrontEnd.h"
#include "InputManager.h"
#include "Mat4x4.h"
#include "Player.h"
#include "Quaternion.h"
#include "Renderer.h"
#include "Span.h"
#include "ThreadPool.h"
#include "Vec3.h"
#include "World.h"
#include "ZColor.h"

namespace ZSharp {

void InitializeGlobals();

void FreeGlobals();

class GameInstance final {
  public:

  GameInstance();
  ~GameInstance();

  GameInstance(const GameInstance& rhs) = delete;
  void operator=(const GameInstance& rhs) = delete;

  void Initialize(bool skipTitleScreen);

  void Tick();

  void TickAudio();

  uint8* GetCurrentFrame();

  void RunBackgroundJobs();

  void WaitForBackgroundJobs();

  private:
  FrontEnd* mFrontEnd = nullptr;
  Player* mPlayer = nullptr;
  World* mWorld = nullptr;
  Renderer* mRenderer = nullptr;
  ThreadPool* mThreadPool = nullptr;

  struct ExtraState {
    size_t mLastFrameTime = 0;
    size_t mLastAudioTime = 0;

    int64 mFrameCount = 0;
    int64 mRotationAmount = 0;
    int64 mRotationSpeed = 4;

    struct {
      bool mPauseTransforms : 1;
      bool mDrawStats : 1;
      bool mVisualizeDepth : 1;
    };
  };

  ExtraState* mExtraState = nullptr;

  ConsoleVariable<void>* mCameraReset = nullptr;

  void PauseTransforms();

  void LoadWorld();

  void LoadFrontEnd();

  void TickWorld();

  void TickFrontEnd();

  void ChangeSpeed(int64 amount);

  void OnKeyDown(uint8 key);

  void OnMiscKeyDown(MiscKey key);

  void FastClearFrameBuffer(Span<uint8> data);

  void FastClearDepthBuffer(Span<uint8> data);

  void ResetCamera();
};

}
