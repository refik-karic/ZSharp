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

float ParametricSolveForT(const float step, const float p0, const float p1);

float PerspectiveLerp(const float p0, const float p1, const float p0Z, const float p1Z, const float t);

void DrawTrianglesFlat(Framebuffer& framebuffer,
    const VertexBuffer& vertexBuffer,
    const IndexBuffer& indexBuffer,
    ZColor color);

void DrawTrianglesWireframe(Framebuffer& framebuffer,
    const VertexBuffer& vertexBuffer,
    const IndexBuffer& indexBuffer);

void DrawRunSlice(Framebuffer& framebuffer,
    const Vec4& p0,
    const Vec4& p1,
    const float* p0Attributes,
    const float* p1Attributes);

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
