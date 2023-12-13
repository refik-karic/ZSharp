#pragma once

#include "Camera.h"
#include "Framebuffer.h"
#include "DepthBuffer.h"
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

  uint8* GetDepth();

  void ToggleRenderMode(RenderMode mode);

  Framebuffer& GetFrameBuffer();

  private:
  Framebuffer mFramebuffer;
  DepthBuffer mDepthBuffer;
  RenderMode mRenderMode = RenderMode::FLAT;

  bool mDepthBufferDirty = false;
};
}
