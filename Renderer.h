#pragma once

#include "Camera.h"
#include "Framebuffer.h"
#include "World.h"

namespace ZSharp {
class Renderer final {
  public:

  Renderer();

  void Initialize();

  Renderer(const Renderer&) = delete;
  void operator=(const Renderer&) = delete;

  void RenderNextFrame(World& world, Camera& camera);

  uint8* GetFrame();

  void FlipRenderMode();

  private:
  Framebuffer mBuffer;

  bool mRenderMode = true;
};
}
