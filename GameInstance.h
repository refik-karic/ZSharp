#pragma once

#include "Camera.h"
#include "DevConsole.h"
#include "FrontEnd.h"
#include "InputManager.h"
#include "Mat4x4.h"
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

  enum class Direction {
    LEFT,
    RIGHT,
    FORWARD,
    BACK
  };

  GameInstance();
  ~GameInstance();

  GameInstance(const GameInstance& rhs) = delete;
  void operator=(const GameInstance& rhs) = delete;

  void Initialize(bool skipTitleScreen);

  void Tick();

  void TickAudio();

  uint8* GetCurrentFrame();

  bool IsDevConsoleOpen() const;

  void RunBackgroundJobs();

  void WaitForBackgroundJobs();

  private:
  FrontEnd* mFrontEnd = nullptr;
  Camera* mCamera = nullptr;
  World* mWorld = nullptr;
  Renderer* mRenderer = nullptr;
  ThreadPool* mThreadPool = nullptr;

  struct ExtraState {
    size_t mLastFrameTime = 0;
    size_t mLastAudioTime = 0;

    int64 mFrameCount = 0;
    int64 mRotationAmount = 0;
    int64 mRotationSpeed = 4;

    bool mPauseTransforms = false;
    bool mDrawStats = true;
    bool mVisualizeDepth = false;
  };

  ExtraState* mExtraState = nullptr;

  DevConsole* mDevConsole = nullptr;

  ConsoleVariable<void>* mCameraReset = nullptr;

  void PauseTransforms();

  void LoadWorld();

  void LoadFrontEnd();

  void TickWorld();

  void TickFrontEnd();

  void MoveCamera(Direction direction);

  void RotateCamera(const Quaternion& quat);

  Vec3 ProjectClick(float x, float y);

  void ChangeSpeed(int64 amount);

  void ResetCamera();

  void OnKeyDown(uint8 key);

  void OnKeyUp(uint8 key);

  void OnMiscKeyDown(MiscKey key);

  void OnMiscKeyUp(MiscKey key);

  void OnMouseMove(int32 oldX, int32 oldY, int32 x, int32 y);

  void FastClearFrameBuffer(Span<uint8> data);

  void FastClearDepthBuffer(Span<uint8> data);
};

}
