#pragma once

#include "Camera.h"
#include "Framebuffer.h"
#include "IInputListener.h"
#include "Mat4x4.h"
#include "Quaternion.h"
#include "Vec3.h"
#include "World.h"
#include "ZColor.h"

namespace ZSharp {
class Renderer final : public IInputListener {
  public:

  enum class Direction {
    LEFT,
    RIGHT,
    FORWARD,
    BACK
  };

  Renderer(size_t width, size_t height, size_t stride);

  Renderer(const Renderer&) = delete;
  void operator=(const Renderer&) = delete;

  uint8* RenderNextFrame();

  void OnKeyDown(uint8 key) override;

  void OnKeyUp(uint8 key) override;

  void OnMouseMove(int32 oldX, int32 oldY, int32 x, int32 y) override;

  private:

  Camera mCamera;
  Framebuffer mBuffer;
  World mWorld;

  int64 mFrameCount = 0;
  int64 mRotationSpeed = 4;
  bool mRenderMode = false;
  bool mPauseTransforms = false;

  void LoadAssets();

  void MoveCamera(Direction direction);

  void RotateCamera(Mat4x4::Axis direction, const float angleDegrees);

  void RotateTrackball(Quaternion quat);

  void ChangeSpeed(int64 amount);

  void FlipRenderMode();

  void PauseTransforms();

  Vec3 ProjectClick(float x, float y);
};
}
