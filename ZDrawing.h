#pragma once

#include "ZBaseTypes.h"

#include "Framebuffer.h"
#include "DepthBuffer.h"
#include "IndexBuffer.h"
#include "Texture.h"
#include "VertexBuffer.h"
#include "ZColor.h"

namespace ZSharp {

void RGBShader(Framebuffer& framebuffer,
  DepthBuffer& depthBuffer,
  const VertexBuffer& vertexBuffer,
  const IndexBuffer& indexBuffer,
  bool wasClipped);

void TextureMappedShader(Framebuffer& framebuffer,
  DepthBuffer& depthBuffer,
  const VertexBuffer& vertexBuffer,
  const IndexBuffer& indexBuffer,
  bool wasClipped,
  const Texture* texture,
  size_t mipLevel);

void WireframeShader(Framebuffer& framebuffer,
  const VertexBuffer& vertexBuffer,
  const IndexBuffer& indexBuffer,
  bool wasClipped,
  ZColor color);

void DrawRunSlice(Framebuffer& framebuffer,
  const float* p0,
  const float* p1,
  ZColor color);

}
