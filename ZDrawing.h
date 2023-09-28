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

FORCE_INLINE float BarycentricArea(const float v1[2], const float v2[2], const float v3[2]);

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
