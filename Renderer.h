﻿#pragma once

#include <memory>

#include "Camera.h"
#include "Framebuffer.h"
#include "IInputListener.h"
#include "Mat4x4.h"
#include "Model.h"
#include "Quaternion.h"
#include "Vec3.h"
#include "ZColor.h"

namespace ZSharp {
class Renderer final : public IInputListener {
  public:

  enum class Direction {
    LEFT,
    RIGHT,
    UP,
    DOWN
  };

  Renderer(std::size_t width, std::size_t height, std::size_t stride);

  Renderer(const Renderer&) = delete;
  void operator=(const Renderer&) = delete;

  std::uint8_t* RenderNextFrame();

  void OnKeyDown(std::uint8_t key) override;

  void OnKeyUp(std::uint8_t key) override;

  void OnMouseMove(std::int32_t oldX, std::int32_t oldY, std::int32_t x, std::int32_t y) override;

  private:

  Camera mCamera;
  Vec3 mCameraPos;

  Framebuffer mBuffer;
  Model mModel;

  std::int64_t mFrameCount = 0;
  std::int64_t mRotationSpeed = 4;
  bool mRenderMode = false;
  bool mPauseTransforms = false;

  std::shared_ptr<IndexBuffer> mIndexBuffer;
  std::shared_ptr<VertexBuffer> mVertexBuffer;

  void MoveCamera(Direction direction, const float amount);

  void RotateCamera(Mat4x4::Axis direction, const float angleDegrees);

  void RotateTrackball(Quaternion quat);

  void ChangeSpeed(std::int64_t amount);

  void FlipRenderMode();

  void PauseTransforms();

  Vec3 ProjectClick(float x, float y);
};
}
