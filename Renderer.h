#pragma once

#include "Camera.h"
#include "Framebuffer.h"
#include "DepthBuffer.h"
#include "World.h"
#include "ZColor.h"

namespace ZSharp {

enum class RenderMode {
  WIREFRAME,
  FILL
};

class Renderer final {
  public:

  Renderer();

  Renderer(const Renderer&) = delete;
  void operator=(const Renderer&) = delete;

  void RenderNextFrame(World& world, Camera& camera);

  uint8* GetFrame();

  uint8* GetDepth();

  void ClearFramebuffer(const ZColor& color);

  void ToggleRenderMode(RenderMode mode);

  Framebuffer& GetFrameBuffer();

  DepthBuffer& GetDepthBuffer();

  private:
  Framebuffer mFramebuffer;
  DepthBuffer mDepthBuffer;
  RenderMode mRenderMode = RenderMode::FILL;

  VertexBuffer mAABBVertexBuffer;
  IndexBuffer mAABBIndexBuffer;

  bool mDepthBufferDirty = false;
};
}
