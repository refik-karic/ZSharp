#pragma once

#include "ZBaseTypes.h"

#include "Framebuffer.h"
#include "DepthBuffer.h"
#include "IndexBuffer.h"
#include "Texture.h"
#include "VertexBuffer.h"
#include "PlatformDefines.h"

namespace ZSharp {

void DrawTrianglesFlatRGB(Framebuffer& framebuffer,
  DepthBuffer& depthBuffer,
  const VertexBuffer& vertexBuffer,
  const IndexBuffer& indexBuffer,
  bool wasClipped);

void DrawTrianglesFlatUV(Framebuffer& framebuffer,
  DepthBuffer& depthBuffer,
  const VertexBuffer& vertexBuffer,
  const IndexBuffer& indexBuffer,
  bool wasClipped,
  const Texture* texture);

void DrawTrianglesWireframe(Framebuffer& framebuffer,
  const VertexBuffer& vertexBuffer,
  const IndexBuffer& indexBuffer,
  bool wasClipped);

void DrawRunSlice(Framebuffer& framebuffer,
    const float* p0,
    const float* p1);

}
