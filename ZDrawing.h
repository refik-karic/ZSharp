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
    std::int32_t x1,
    std::int32_t y1,
    std::int32_t x2,
    std::int32_t y2,
    ZColor color);

void TracePrimitive(GlobalEdgeTable& edgeTable, 
    std::array<std::int32_t, 2>& p1, 
    std::array<std::int32_t, 2>& p2, 
    std::array<std::int32_t, 2>& p3, 
    ZColor color, 
    std::size_t primitiveIndex);

void TraceLine(GlobalEdgeTable& edgeTable,
    std::int32_t x1,
    std::int32_t y1,
    std::int32_t x2,
    std::int32_t y2,
    ZColor color,
    std::size_t primitiveIndex);

}
