#pragma once

#include <cstdint>
#include <cstddef>

#include <array>

#include "Framebuffer.h"
#include "GlobalEdgeTable.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "ZColor.h"

namespace ZSharp {

void DrawTrianglesFlat(Framebuffer& framebuffer,
    const VertexBuffer& vertexBuffer,
    const IndexBuffer& indexBuffer,
    ZColor color);

void DrawTrianglesWireframe(Framebuffer& framebuffer,
    const VertexBuffer& vertexBuffer,
    const IndexBuffer& indexBuffer,
    ZColor color);

void DrawRunSlice(Framebuffer& framebuffer,
    int32_t x1,
    int32_t y1,
    int32_t x2,
    int32_t y2,
    ZColor color);

void TracePrimitive(GlobalEdgeTable& edgeTable, 
    std::array<int32_t, 2>& p1, 
    std::array<int32_t, 2>& p2, 
    std::array<int32_t, 2>& p3, 
    ZColor color, 
    size_t primitiveIndex);

void TraceLine(GlobalEdgeTable& edgeTable,
    int32_t x1,
    int32_t y1,
    int32_t x2,
    int32_t y2,
    ZColor color,
    size_t primitiveIndex);

}
