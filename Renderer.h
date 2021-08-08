#pragma once

#include <memory>

#include "Camera.h"
#include "Framebuffer.h"
#include "IInputListener.h"
#include "Mat4x4.h"
#include "Model.h"
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

  private:

  Camera<float> mCamera;
  Vec3<float> mCameraPos;

  Framebuffer mBuffer;
  Model<float> mModel;

  std::int64_t mFrameCount = 0;
  std::int64_t mRotationSpeed = 4;
  bool mRenderMode = false;
  bool mPauseTransforms = false;

  std::shared_ptr<IndexBuffer> mIndexBuffer;
  std::shared_ptr<VertexBuffer<float>> mVertexBuffer;

  void MoveCamera(Direction direction, float amount);

  void RotateCamera(Mat4x4<float>::Axis direction, float angleDegrees);

  void ChangeSpeed(std::int64_t amount);

  void FlipRenderMode();

  void PauseTransforms();
};
}
