#pragma once

#include "Camera.h"
#include "DevConsole.h"
#include "Mat4x4.h"
#include "Quaternion.h"
#include "Renderer.h"
#include "Vec3.h"
#include "World.h"
#include "ZColor.h"
#include "PlatformAudio.h"
#include "MP3.h"

namespace ZSharp {

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

  void Initialize();

  void Tick();

  void TickAudio();

  uint8* GetCurrentFrame();

  bool IsDevConsoleOpen() const;

  private:
  Camera mCamera;
  World mWorld;
  Renderer mRenderer;
  PlatformAudioDevice* mAudioDevice = nullptr;
  MP3::PCMAudioFloat mAmbientTrack;
  size_t mAudioPosition = 0;

  size_t mLastFrameTime = 0;
  size_t mLastAudioTime = 0;

  int64 mFrameCount = 0;
  int64 mRotationAmount = 0;
  int64 mRotationSpeed = 4;

  bool mPauseTransforms = false;
  bool mDrawStats = true;
  bool mVisualizeDepth = false;

  // TODO: ifdef this out in release builds at some point.
  DevConsole mDevConsole;

  void PauseTransforms();

  void LoadAssets();

  void MoveCamera(Direction direction);

  void RotateCamera(Mat4x4::Axis direction, const float angleDegrees);

  void RotateTrackball(const Quaternion& quat);

  Vec3 ProjectClick(float x, float y);

  void ChangeSpeed(int64 amount);

  void OnKeyDown(uint8 key);

  void OnKeyUp(uint8 key);

  void OnMouseMove(int32 oldX, int32 oldY, int32 x, int32 y);

};

}
