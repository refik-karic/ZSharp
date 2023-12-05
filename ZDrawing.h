#pragma once

#include "ZBaseTypes.h"

#include "Framebuffer.h"
#include "DepthBuffer.h"
#include "IndexBuffer.h"
#include "ShadingMode.h"
#include "Texture.h"
#include "VertexBuffer.h"
#include "PlatformDefines.h"

namespace ZSharp {

void DrawTrianglesFlat(Framebuffer& framebuffer,
    DepthBuffer& depthBuffer,
    const VertexBuffer& vertexBuffer,
    const IndexBuffer& indexBuffer,
    const ShadingModeOrder& order,
    const Texture* texture);

void DrawTrianglesWireframe(Framebuffer& framebuffer,
    const VertexBuffer& vertexBuffer,
    const IndexBuffer& indexBuffer);

void DrawRunSlice(Framebuffer& framebuffer,
    const float* p0,
    const float* p1);

}
