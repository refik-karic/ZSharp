#pragma once

#include "ZBaseTypes.h"
#include "ZString.h"
#include "Texture.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

namespace ZSharp {

// Denotes 32-bit word width.
// i.e. FourWide = 4 32-bit words = 128bit vector register
enum class SIMDLaneWidth {
  Four,
  Eight,
  Sixteen
};

bool PlatformSupportsSIMDLanes(SIMDLaneWidth width);

String PlatformCPUVendor();

String PlatformCPUBrand();

void Aligned_128Add(const float* a, const float* b, float* dest);

void Aligned_128Sub(const float* a, const float* b, float* dest);

void Aligned_128MulByValue(const float* a, const float b, float* dest);

float Aligned_128MulSum(const float* a, const float* b);

/*
NOTE: This does NOT behave like C's memset() which casts value to a byte.
      It will attempt to copy all 4 bytes of value in the fastest way possible.
*/
void Aligned_Memset(void* __restrict dest, uint32 value, const size_t numBytes);

void Unaligned_128Add(const float* a, const float* b, float* dest);

void Unaligned_128Sub(const float* a, const float* b, float* dest);

void Unaligned_128MulByValue(const float* a, const float b, float* dest);

float Unaligned_128MulSum(const float* a, const float* b);

void Aligned_Mat4x4Transform(const float matrix[4][4], float* data, size_t stride, size_t length);

void Aligned_Vec4Homogenize(float* data, size_t stride, size_t length);

void Aligned_BackfaceCull(IndexBuffer& indexBuffer, const VertexBuffer& vertexBuffer, const float viewer[3]);

void Unaligned_AABB(const float* vertices, size_t numVertices, size_t stride, float outMin[4], float outMax[4]);

void Unaligned_FlatShadeRGB(const float* v1, const float* v2, const float* v3,
  const float maxWidth, const float maxHeight, uint8* framebuffer, float* depthBuffer);

void Unaligned_FlatShadeUVs(const float* v1, const float* v2, const float* v3,
  const float maxWidth, const float maxHeight, uint8* framebuffer, float* depthBuffer, const Texture* texture);

}
