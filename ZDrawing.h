#pragma once

#include "ZBaseTypes.h"
#include "ZBaseTypes.h"

#include "FixedArray.h"
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
    int32 x1,
    int32 y1,
    int32 x2,
    int32 y2,
    ZColor color);

void TracePrimitive(GlobalEdgeTable& edgeTable, 
    FixedArray<size_t, 2>& p1,
    FixedArray<size_t, 2>& p2,
    FixedArray<size_t, 2>& p3,
    ZColor color, 
    size_t primitiveIndex);

void TraceLine(GlobalEdgeTable& edgeTable,
    int32 x1,
    int32 y1,
    int32 x2,
    int32 y2,
    ZColor color,
    size_t primitiveIndex);

}
