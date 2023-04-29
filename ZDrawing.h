#pragma once

#include "ZBaseTypes.h"

#include "FixedArray.h"
#include "Framebuffer.h"
#include "GlobalEdgeTable.h"
#include "IndexBuffer.h"
#include "ShadingMode.h"
#include "VertexBuffer.h"

namespace ZSharp {

float PerspectiveLerp(const float p0, 
  const float p1, 
  const float p0Z, 
  const float invP0z,
  const float p1Z, 
  const float invP1z,
  const float t);

void DrawTrianglesFlat(Framebuffer& framebuffer,
    const VertexBuffer& vertexBuffer,
    const IndexBuffer& indexBuffer,
    const ShadingModeOrder& order);

void DrawTrianglesWireframe(Framebuffer& framebuffer,
    const VertexBuffer& vertexBuffer,
    const IndexBuffer& indexBuffer);

void DrawRunSlice(Framebuffer& framebuffer,
    const float* p0,
    const float* p1);

void TraceLine(GlobalEdgeTable& edgeTable,
  const float* p0,
  const float* p1,
  size_t primitiveIndex,
  size_t attributeStride);

}
