#pragma once

#include "ZBaseTypes.h"
#include "ZString.h"
#include "Texture.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ZColor.h"

namespace ZSharp {

// Denotes 32-bit word width.
// i.e. FourWide = 4 32-bit words = 128bit vector register
enum class SIMDLaneWidth {
  Four,
  Eight,
  Sixteen
};

bool PlatformSupportsSIMDLanes(SIMDLaneWidth width);

size_t PlatformAlignmentGranularity();

String PlatformCPUVendor();

String PlatformCPUBrand();

void Aligned_128Add(const float* a, const float* b, float* dest);

void Aligned_128AddInPlace(float* a, const float* b);

void Aligned_128Sub(const float* a, const float* b, float* dest);

void Aligned_128SubInPlace(float* a, const float* b);

void Aligned_128MulByValue(const float* a, const float b, float* dest);

void Aligned_128MulByValueInPlace(float* a, const float b);

float Aligned_128MulSum(const float* a, const float* b);

/*
NOTE: This does NOT behave like C's memset() which casts value to a byte.
      It will attempt to copy all 4 bytes of value in the fastest way possible.
*/
void Aligned_Memset(void* __restrict dest, uint32 value, const size_t numBytes);

/*
This attempts to use the fastest available uArch routine for copying large blocks of data.
*/
void Aligned_Memcpy(void* __restrict dest, const void* __restrict src, size_t numBytes);

void Unaligned_128Add(const float* a, const float* b, float* dest);

void Unaligned_128Sub(const float* a, const float* b, float* dest);

void Unaligned_128MulByValue(const float* a, const float b, float* dest);

float Unaligned_128MulSum(const float* a, const float* b);

float Unaligned_Vec3Length(const float* a);

void Unaligned_Vec3Normalize(float* a);

float Unaligned_Vec4Length(const float* a);

void Unaligned_Vec4Normalize(float* a);

void Unaligned_Vec4Homogenize(float* a);

void Unaligned_Vec4HomogenizeToVec3(const float* __restrict a, float* __restrict b);

void Unaligned_Mat4x4Scale(const float inMatrix[4][4], float outMatrix[4][4], float scalar);

void Unaligned_Mat4x4Transpose(const float inMatrix[4][4], float outMatrix[4][4]);

void Unaligned_Mat4x4Vec4Transform(const float matrix[4][4], const float* __restrict a, float* __restrict b);

void Unaligned_Mat4x4Vec4TransformInPlace(const float matrix[4][4], float* a);

float Unaligned_ParametricLinePlaneIntersection(const float start[4], const float end[4], const float edgeNormal[4], const float edgePoint[4]);

bool Unaligned_InsidePlane(const float point[4], const float clipEdge[4], const float normal[4]);

void Unaligned_ParametricVector4D(const float point, const float start[4], const float end[4], float outVec[4]);

void Unaligned_LerpAttribute(const float attributeA[4], const float attributeB[4], float outAttribute[4], float parametricT);

void Unaligned_Mat4x4Mul(const float* a, const float* b, float* result);

void Unaligned_Mat4x4MulInPlace(float* a, const float* b);

void Unaligned_Mat4x4Mul_Combine(const float** inMats, size_t size, float* result);

void Unaligned_RGBXToBGRA(const uint8* rgb, uint8* rgba, size_t rgbBytes);

void Unaligned_BGRToBGRA(const uint8* rgb, uint8* rgba, size_t rgbBytes);

void Unaligned_RGBAToBGRA(uint32* image, size_t width, size_t height);

typedef void (*BilinearScaleImageFunc)(uint8* __restrict source, size_t sourceWidth, size_t sourceHeight, uint8* __restrict dest, size_t destWidth, size_t destHeight);

extern BilinearScaleImageFunc BilinearScaleImageImpl;

void Unaligned_BilinearScaleImage_SSE(uint8* __restrict source, size_t sourceWidth, size_t sourceHeight, uint8* __restrict dest, size_t destWidth, size_t destHeight);

void Unaligned_BilinearScaleImage_AVX(uint8* __restrict source, size_t sourceWidth, size_t sourceHeight, uint8* __restrict dest, size_t destWidth, size_t destHeight);

typedef void (*GenerateMipLevelFunc)(uint8* __restrict nextMip, size_t nextWidth, size_t nextHeight, uint8* __restrict lastMip, size_t lastWidth, size_t lastHeight);

extern GenerateMipLevelFunc GenerateMipLevelImpl;

void Unaligned_GenerateMipLevel_SSE(uint8* __restrict nextMip, size_t nextWidth, size_t nextHeight, uint8* __restrict lastMip, size_t lastWidth, size_t lastHeight);

void Unaligned_GenerateMipLevel_AVX(uint8* __restrict nextMip, size_t nextWidth, size_t nextHeight, uint8* __restrict lastMip, size_t lastWidth, size_t lastHeight);

typedef void (*DrawDebugTextFunc)(const uint8 lut[128][8], const String& message, size_t x, size_t y, uint8* buffer, size_t width, const ZColor& color);

extern DrawDebugTextFunc DrawDebugTextImpl;

void Unaligned_DrawDebugText_SSE(const uint8 lut[128][8], const String& message, size_t x, size_t y, uint8* buffer, size_t width, const ZColor& color);

void Unaligned_DrawDebugText_AVX(const uint8 lut[128][8], const String& message, size_t x, size_t y, uint8* buffer, size_t width, const ZColor& color);

void Aligned_Mat4x4Transform(const float matrix[4][4], float* __restrict data, int32 stride, int32 length);

typedef void (*DepthBufferVisualizeFunc)(float* buffer, size_t width, size_t height);

extern DepthBufferVisualizeFunc DepthBufferVisualizeImpl;

void Aligned_DepthBufferVisualize_SSE(float* buffer, size_t width, size_t height);

void Aligned_DepthBufferVisualize_AVX(float* buffer, size_t width, size_t height);

void Aligned_Vec4Homogenize(float* data, int32 stride, int32 length);

typedef void (*BlendBuffersFunc)(uint32* __restrict devBuffer, uint32* __restrict frameBuffer, size_t width, size_t height, float opacity);

extern BlendBuffersFunc BlendBuffersImpl;

void Unaligned_BlendBuffers_SSE(uint32* __restrict devBuffer, uint32* __restrict frameBuffer, size_t width, size_t height, float opacity);

void Unaligned_BlendBuffers_AVX(uint32* __restrict devBuffer, uint32* __restrict frameBuffer, size_t width, size_t height, float opacity);

void Aligned_BackfaceCull(IndexBuffer& indexBuffer, const VertexBuffer& vertexBuffer);

void Aligned_WindowTransform(float* data, int32 stride, int32 length, const float windowTransform0[3], const float windowTransform1[3], const float width, const float height);

void Aligned_TransformDirectScreenSpace(float* data, int32 stride, int32 length,
  const float matrix[4][4], const float windowTransform0[3], const float windowTransform1[3],
  const float width, const float height);

void Aligned_HomogenizeTransformScreenSpace(float* data, int32 stride, int32 length,
  const float windowTransform0[3], const float windowTransform1[3],
  const float width, const float height);

typedef void (*CalculateAABBFunc)(const float* vertices, size_t numVertices, size_t stride, float outMin[4], float outMax[4]);

extern CalculateAABBFunc CalculateAABBImpl;

void Unaligned_AABB_SSE(const float* vertices, size_t numVertices, size_t stride, float outMin[4], float outMax[4]);

void Unaligned_AABB_AVX(const float* vertices, size_t numVertices, size_t stride, float outMin[4], float outMax[4]);

bool Unaligned_AABB_Intersects(const float* aMin, const float* aMax, const float* bMin, const float* bMax);

void Unaligned_AABB_TransformAndRealign(const float* inMin, const float* inMax, float* outMin, float* outMax, const float* matrix);

typedef void (*RGBShaderFunc)(const float* __restrict vertices, const int32* __restrict indices, const int32 end,
  const float maxWidth, uint8* __restrict framebuffer, float* __restrict depthBuffer);

extern RGBShaderFunc RGBShaderImpl;

void Unaligned_Shader_RGB_SSE(const float* __restrict vertices, const int32* __restrict indices, const int32 end,
  const float maxWidth, uint8* __restrict framebuffer, float* __restrict depthBuffer);

void Unaligned_Shader_RGB_AVX(const float* __restrict vertices, const int32* __restrict indices, const int32 end,
  const float maxWidth, uint8* __restrict framebuffer, float* __restrict depthBuffer);

typedef void (*UVShaderFunc)(const float* __restrict vertices, const int32* __restrict indices, const int32 end,
  const float maxWidth, uint8* __restrict framebuffer, float* __restrict depthBuffer, const Texture* __restrict texture, size_t mipLevel);

extern UVShaderFunc UVShaderImpl;

void Unaligned_Shader_UV_SSE(const float* __restrict vertices, const int32* __restrict indices, const int32 end,
  const float maxWidth, uint8* __restrict framebuffer, float* __restrict depthBuffer, const Texture* __restrict texture, size_t mipLevel);

void Unaligned_Shader_UV_AVX(const float* __restrict vertices, const int32* __restrict indices, const int32 end,
  const float maxWidth, uint8* __restrict framebuffer, float* __restrict depthBuffer, const Texture* __restrict texture, size_t mipLevel);

}
