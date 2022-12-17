#pragma once

#include "Camera.h"
#include "Framebuffer.h"
#include "World.h"

namespace ZSharp {

enum class RenderMode {
  WIREFRAME,
  FLAT
};

class Renderer final {
  public:

  Renderer();

  Renderer(const Renderer&) = delete;
  void operator=(const Renderer&) = delete;

  void RenderNextFrame(World& world, Camera& camera);

  uint8* GetFrame();

  void ToggleRenderMode(RenderMode mode);

  private:
  Framebuffer mBuffer;
  RenderMode mRenderMode = RenderMode::FLAT;
};
}
