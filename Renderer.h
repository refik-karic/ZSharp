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

  uint8_t* RenderNextFrame();

  void OnKeyDown(uint8_t key) override;

  void OnKeyUp(uint8_t key) override;

  void OnMouseMove(int32_t oldX, int32_t oldY, int32_t x, int32_t y) override;

  private:

  Camera mCamera;
  Framebuffer mBuffer;
  World mWorld;

  int64_t mFrameCount = 0;
  int64_t mRotationSpeed = 4;
  bool mRenderMode = false;
  bool mPauseTransforms = false;

  void MoveCamera(Direction direction, const float amount);

  void RotateCamera(Mat4x4::Axis direction, const float angleDegrees);

  void RotateTrackball(Quaternion quat);

  void ChangeSpeed(int64_t amount);

  void FlipRenderMode();

  void PauseTransforms();

  float ProjectClick(float x, float y);
};
}
