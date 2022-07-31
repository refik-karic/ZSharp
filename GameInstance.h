#pragma once

#include "Camera.h"
#include "IInputListener.h"
#include "Mat4x4.h"
#include "Quaternion.h"
#include "Renderer.h"
#include "Vec3.h"
#include "World.h"
#include "ZColor.h"

namespace ZSharp {

class GameInstance final : public IInputListener {
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
  GameInstance(const GameInstance&& rhs) = delete;
  void operator=(const GameInstance& rhs) = delete;

  void Initialize();

  void Tick();

  uint8* GetCurrentFrame();

  void OnKeyDown(uint8 key) override;

  void OnKeyUp(uint8 key) override;

  void OnMouseMove(int32 oldX, int32 oldY, int32 x, int32 y) override;

  private:
  Camera mCamera;
  World mWorld;
  Renderer mRenderer;

  int64 mFrameCount = 0;
  int64 mRotationSpeed = 4;

  bool mPauseTransforms = false;

  void PauseTransforms();

  void LoadAssets();

  void MoveCamera(Direction direction);

  void RotateCamera(Mat4x4::Axis direction, const float angleDegrees);

  void RotateTrackball(Quaternion quat);

  Vec3 ProjectClick(float x, float y);

  void ChangeSpeed(int64 amount);

};

}
