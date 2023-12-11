#ifdef PLATFORM_WINDOWS

#include "PlatformIntrinsics.h"

#include "ZBaseTypes.h"
#include "ZAssert.h"

#include <cstring>

#ifdef HW_PLATFORM_X86

#include <immintrin.h>
#include <intrin.h>

int* CPUIDSection00() {
  static int buffer[4] = { 0, 0, 0, 0 };

  if (buffer[0] == 0) {
    __cpuid(buffer, 0x00);

    // [0] = EAX
    // [1] = EBX
    // [2] = ECX
    // [3] = EDX
  }

  return buffer;
}

int* CPUIDSection01() {
  static int buffer[4] = { 0, 0, 0, 0 };

  if (buffer[2] == 0) {
    __cpuid(buffer, 0x01);

    // [0] = EAX
    // [1] = EBX
    // [2] = ECX
    // [3] = EDX
  }

  return buffer;
}

int* CPUIDSection07() {
  static int buffer[4] = { 0, 0, 0, 0 };

  if (buffer[2] == 0) {
    __cpuid(buffer, 0x07);

    // [0] = EAX
    // [1] = EBX
    // [2] = ECX
    // [3] = EDX
  }

  return buffer;
}

int* CPUIDSectionBrand() {
  static int buffer[12] = { 0, 0, 0, 0,
                            0, 0, 0, 0,
                            0, 0, 0, 0};

  if (buffer[0] == 0) {
    __cpuid(buffer, 0x80000002);
    __cpuid(buffer + 4, 0x80000003);
    __cpuid(buffer + 8, 0x80000004);

    // [0] = EAX
    // [1] = EBX
    // [2] = ECX
    // [3] = EDX
  }

  return buffer;
}

namespace ZSharp {

bool PlatformSupportsSIMDLanes(SIMDLaneWidth width) {
  switch (width) {
#if 0
    case SIMDWidth::Four:
    {
      // Checks for SSE2.
      int* bits = CPUIDSection01();
      return bits[3] & (1 << 26);
    }
    break;
#endif
    case SIMDLaneWidth::Four: // SSE4
    {
      int* bits = CPUIDSection01();
      return bits[2] & (1 << 19);
    }
    break;
    case SIMDLaneWidth::Eight: // AVX2
    {
      int* bits = CPUIDSection07();
      return bits[1] & (1 << 5);
    }
    break;
    case SIMDLaneWidth::Sixteen: // AVX512
    {
      int* bits = CPUIDSection07();
      return bits[1] & (1 << 16);
    }
    break;
  }

  return false;
}

String PlatformCPUVendor() {
  String vendor;

  const char* vendorString = (const char*)CPUIDSection00() + 4;
  vendor.Append(vendorString, 0, 4);
  vendor.Append(vendorString, 8, 4);
  vendor.Append(vendorString, 4, 4);

  return vendor;
}

String PlatformCPUBrand() {
  String brand;

  const char* brandString = (const char*)CPUIDSectionBrand();
  brand.Append(brandString, 0, 16);
  brand.Append(brandString, 16, 16);
  brand.Append(brandString, 32, 16);

  return brand;
}

void Aligned_128Add(const float* a, const float* b, float* dest) {
  _mm_store_ps(dest, _mm_add_ps(_mm_load_ps(a), _mm_load_ps(b)));
}

void Aligned_128Sub(const float* a, const float* b, float* dest) {
  _mm_store_ps(dest, _mm_sub_ps(_mm_load_ps(a), _mm_load_ps(b)));
}

void Aligned_128MulByValue(const float* a, const float b, float* dest) {
  _mm_store_ps(dest, _mm_mul_ps(_mm_load_ps(a), _mm_set_ps1(b)));
}

float Aligned_128MulSum(const float* a, const float* b) {
  __m128 mulResult = _mm_mul_ps(_mm_load_ps(a), _mm_load_ps(b));
  mulResult = _mm_hadd_ps(mulResult, mulResult);
  mulResult = _mm_hadd_ps(mulResult, mulResult);
  return _mm_cvtss_f32(mulResult);
}

void Aligned_Memset(void* __restrict dest, uint32 value, const size_t numBytes) {
  __stosd((unsigned long*)dest, value, numBytes / 4);

#if 0
  if (PlatformSupportsSIMDLanes(SIMDLaneWidth::Sixteen) && ((numBytes % sizeof(__m512i)) == 0)) {
    __m512i repData;

    for (size_t i = 0; i < sizeof(repData.m512i_u32) / sizeof(uint32); ++i) {
      repData.m512i_u32[i] = value;
    }

    __m512i* nextDest = reinterpret_cast<__m512i*>(dest);
    for (size_t i = 0; i < numBytes; i += sizeof(__m512i)) {
      _mm512_store_epi32(nextDest, repData);
      ++nextDest;
    }
  }
  else if((numBytes % sizeof(uint32)) == 0) {
    uint32* data = (uint32*)dest;
    for (size_t i = 0; i * sizeof(uint32) < numBytes; ++i) {
      data[i] = value;
    }
  }
  else {
    ZAssert(false); // numBytes is not a valid multiple.
  }
#endif
}

void Unaligned_128Add(const float* a, const float* b, float* dest) {
  _mm_storeu_ps(dest, _mm_add_ps(_mm_loadu_ps(a), _mm_loadu_ps(b)));
}

void Unaligned_128Sub(const float* a, const float* b, float* dest) {
  _mm_storeu_ps(dest, _mm_sub_ps(_mm_loadu_ps(a), _mm_loadu_ps(b)));
}

void Unaligned_128MulByValue(const float* a, const float b, float* dest) {
  _mm_storeu_ps(dest, _mm_mul_ps(_mm_loadu_ps(a), _mm_set_ps1(b)));
}

float Unaligned_128MulSum(const float* a, const float* b) {
  __m128 mulResult = _mm_mul_ps(_mm_loadu_ps(a), _mm_loadu_ps(b));
  mulResult = _mm_hadd_ps(mulResult, mulResult);
  mulResult = _mm_hadd_ps(mulResult, mulResult);
  return _mm_cvtss_f32(mulResult);
}

void Unaligned_Mat4x4Mul(const float* a, const float* b, float* result) {
  __m128 a0 = _mm_set_ps1(a[0]);
  __m128 a1 = _mm_set_ps1(a[1]);
  __m128 a2 = _mm_set_ps1(a[2]);
  __m128 a3 = _mm_set_ps1(a[3]);

  __m128 a4 = _mm_set_ps1(a[4]);
  __m128 a5 = _mm_set_ps1(a[5]);
  __m128 a6 = _mm_set_ps1(a[6]);
  __m128 a7 = _mm_set_ps1(a[7]);

  __m128 a8 = _mm_set_ps1(a[8]);
  __m128 a9 = _mm_set_ps1(a[9]);
  __m128 a10 = _mm_set_ps1(a[10]);
  __m128 a11 = _mm_set_ps1(a[11]);

  __m128 a12 = _mm_set_ps1(a[12]);
  __m128 a13 = _mm_set_ps1(a[13]);
  __m128 a14 = _mm_set_ps1(a[14]);
  __m128 a15 = _mm_set_ps1(a[15]);

  __m128 br0 = _mm_loadu_ps(b);
  __m128 br1 = _mm_loadu_ps(b + 4);
  __m128 br2 = _mm_loadu_ps(b + 8);
  __m128 br3 = _mm_loadu_ps(b + 12);

  __m128 result0 = _mm_mul_ps(a0, br0);
  __m128 result1 = _mm_mul_ps(a1, br1);
  __m128 result2 = _mm_mul_ps(a2, br2);
  __m128 result3 = _mm_mul_ps(a3, br3);

  __m128 result4 = _mm_mul_ps(a4, br0);
  __m128 result5 = _mm_mul_ps(a5, br1);
  __m128 result6 = _mm_mul_ps(a6, br2);
  __m128 result7 = _mm_mul_ps(a7, br3);

  __m128 result8 = _mm_mul_ps(a8, br0);
  __m128 result9 = _mm_mul_ps(a9, br1);
  __m128 result10 = _mm_mul_ps(a10, br2);
  __m128 result11 = _mm_mul_ps(a11, br3);

  __m128 result12 = _mm_mul_ps(a12, br0);
  __m128 result13 = _mm_mul_ps(a13, br1);
  __m128 result14 = _mm_mul_ps(a14, br2);
  __m128 result15 = _mm_mul_ps(a15, br3);

  _mm_store_ps(result, _mm_add_ps(_mm_add_ps(_mm_add_ps(result0, result1), result2), result3));
  _mm_store_ps(result + 4, _mm_add_ps(_mm_add_ps(_mm_add_ps(result4, result5), result6), result7));
  _mm_store_ps(result + 8, _mm_add_ps(_mm_add_ps(_mm_add_ps(result8, result9), result10), result11));
  _mm_store_ps(result + 12, _mm_add_ps(_mm_add_ps(_mm_add_ps(result12, result13), result14), result15));
}

void Aligned_Mat4x4Transform(const float matrix[4][4], float* data, size_t stride, size_t length) {
  if (PlatformSupportsSIMDLanes(SIMDLaneWidth::Four)) {
    /*
      We preload 4 registers with all of the matrix X, Y, Z, W values.
      We also broadcast each vector component (X, Y, Z, W) to 4 registers.

      This saves us some arithmetic. We only have to perform 4 multiplies and 4 adds per vertex in the end.
    */

    __m128 matrixX = _mm_set_ps(matrix[3][0], matrix[2][0], matrix[1][0], matrix[0][0]);
    __m128 matrixY = _mm_set_ps(matrix[3][1], matrix[2][1], matrix[1][1], matrix[0][1]);
    __m128 matrixZ = _mm_set_ps(matrix[3][2], matrix[2][2], matrix[1][2], matrix[0][2]);
    __m128 matrixW = _mm_set_ps(matrix[3][3], matrix[2][3], matrix[1][3], matrix[0][3]);

    for (size_t i = 0; i < length; i += stride) {
      float* vecData = data + i;

      __m128 vecX = _mm_set_ps1(vecData[0]);
      __m128 vecY = _mm_set_ps1(vecData[1]);
      __m128 vecZ = _mm_set_ps1(vecData[2]);
      __m128 vecW = _mm_set_ps1(vecData[3]);

      vecX = _mm_mul_ps(matrixX, vecX);
      vecY = _mm_mul_ps(matrixY, vecY);
      vecZ = _mm_mul_ps(matrixZ, vecZ);
      vecW = _mm_mul_ps(matrixW, vecW);

      __m128 result = _mm_add_ps(vecX, vecY);
      result = _mm_add_ps(result, vecZ);
      result = _mm_add_ps(result, vecW);

      _mm_store_ps(vecData, result);
    }
  }
  else {
    for (size_t i = 0; i < length; i += stride) {
      float* vec = (data + i);

      float xyzw[4];
      xyzw[0] = (matrix[0][0] * vec[0]) + (matrix[0][1] * vec[1]) + (matrix[0][2] * vec[2]) + (matrix[0][3] * vec[3]);
      xyzw[1] = (matrix[1][0] * vec[0]) + (matrix[1][1] * vec[1]) + (matrix[1][2] * vec[2]) + (matrix[1][3] * vec[3]);
      xyzw[2] = (matrix[2][0] * vec[0]) + (matrix[2][1] * vec[1]) + (matrix[2][2] * vec[2]) + (matrix[2][3] * vec[3]);
      xyzw[3] = (matrix[3][0] * vec[0]) + (matrix[3][1] * vec[1]) + (matrix[3][2] * vec[2]) + (matrix[3][3] * vec[3]);

      memcpy(data + i, xyzw, sizeof(xyzw));
    }
  }
}

void Aligned_Vec4Homogenize(float* data, size_t stride, size_t length) {
  if (PlatformSupportsSIMDLanes(SIMDLaneWidth::Four)) {
    for (size_t i = 0; i < length; i += stride) {
      float* nextVec = data + i;
      float perspectiveTerm = nextVec[3];
      _mm_store_ps(nextVec, _mm_div_ps(_mm_load_ps(nextVec), _mm_set_ps1(perspectiveTerm)));
      nextVec[3] = perspectiveTerm;
    }
  }
  else {
    for (size_t i = 0; i < length; i += stride) {
      float* vec = data + i;
      const float invDivisor = 1.f / vec[3];
      vec[0] *= invDivisor;
      vec[1] *= invDivisor;
      vec[2] *= invDivisor;
    }
  }
}

void Aligned_BackfaceCull(IndexBuffer& indexBuffer, const VertexBuffer& vertexBuffer, const float viewer[3]) {
  __m128 view = _mm_loadu_ps(viewer);
  
  size_t* indexData = indexBuffer.GetInputData();
  const float* vertexData = vertexBuffer[0];
  const size_t vertexStride = vertexBuffer.GetStride();

  for (size_t i = indexBuffer.GetIndexSize(); i >= 3; i -= 3) {
    size_t i1 = indexData[i - 3] * vertexStride;
    size_t i2 = indexData[i - 2] * vertexStride;
    size_t i3 = indexData[i - 1] * vertexStride;
    __m128 v1 = _mm_load_ps(vertexData + i1);
    __m128 v2 = _mm_load_ps(vertexData + i2);
    __m128 v3 = _mm_load_ps(vertexData + i3);

    __m128 p1p0 = _mm_sub_ps(v2, v1);
    __m128 p2p0 = _mm_sub_ps(v3, v1);

    __m128 p1p0Shuffled0 = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(p1p0), 0b11001001));
    __m128 p1p0Shuffled1 = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(p1p0), 0b11010010));

    __m128 p2p0Shuffled0 = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(p2p0), 0b11010010));
    __m128 p2p0Shuffled1 = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(p2p0), 0b11001001));

    __m128 normal = _mm_sub_ps(_mm_mul_ps(p1p0Shuffled0, p2p0Shuffled0), _mm_mul_ps(p1p0Shuffled1, p2p0Shuffled1));

    __m128 dotIntermediate = _mm_mul_ps(_mm_sub_ps(v1, view), normal);
    float dotResult = dotIntermediate.m128_f32[0] + dotIntermediate.m128_f32[1] + dotIntermediate.m128_f32[2];

    if (dotResult > 0.f) {
      indexBuffer.RemoveTriangle(i - 3);
    }
  }
}

void Unaligned_AABB(const float* vertices, size_t numVertices, size_t stride, float outMin[4], float outMax[4]) {
  /*
  Note that we don't do any kind of CPUID check here.
  minps/maxps are available since the first version of SSE.
  */
  
  __m128 min = _mm_loadu_ps(outMin);
  __m128 max = _mm_loadu_ps(outMax);
  
  for (size_t i = 0; i < numVertices; i += stride) {
    __m128 vertex = _mm_loadu_ps(vertices + i);
    min = _mm_min_ps(min, vertex);
    max = _mm_max_ps(max, vertex);
  }

  _mm_storeu_ps(outMin, min);
  _mm_storeu_ps(outMax, max);
}

void Unaligned_FlatShadeRGB(const float* v1, const float* v2, const float* v3, const float maxWidth, const float maxHeight, uint8* framebuffer, float* depthBuffer) {
  __m128 min = _mm_set_ps(v1[0], v1[1], 0.f, 0.f);
  __m128 min1 = _mm_set_ps(v2[0], v2[1], 0.f, 0.f);
  __m128 min2 = _mm_set_ps(v3[0], v3[1], 0.f, 0.f);

  __m128 max = _mm_set_ps(v1[0], v1[1], 0.f, 0.f);
  __m128 max1 = _mm_set_ps(v2[0], v2[1], 0.f, 0.f);
  __m128 max2 = _mm_set_ps(v3[0], v3[1], 0.f, 0.f);

  min = _mm_min_ps(_mm_min_ps(min, min1), min2);
  max = _mm_max_ps(_mm_max_ps(max, max1), max2);

  min = _mm_floor_ps(min);
  max = _mm_ceil_ps(max);

  min = _mm_max_ps(min, _mm_set_ps1(0.f));
  min = _mm_min_ps(min, _mm_set_ps(maxWidth, maxHeight, 0.f, 0.f));
  max = _mm_max_ps(max, _mm_set_ps1(0.f));
  max = _mm_min_ps(max, _mm_set_ps(maxWidth, maxHeight, 0.f, 0.f));

  __m128 invArea = _mm_set_ps1(1.f / ((v3[0] - v1[0]) * (v2[1] - v1[1]) - ((v3[1] - v1[1]) * (v2[0] - v1[0]))));

  __m128 invVert = _mm_mul_ps(_mm_set_ps(v1[3], v2[3], v3[3], 0.f), invArea);
  __m128 invAttr0 = _mm_mul_ps(_mm_set_ps(v1[4], v2[4], v3[4], 0.f), invArea);
  __m128 invAttr1 = _mm_mul_ps(_mm_set_ps(v1[5], v2[5], v3[5], 0.f), invArea);
  __m128 invAttr2 = _mm_mul_ps(_mm_set_ps(v1[6], v2[6], v3[6], 0.f), invArea);

  const size_t sMinX = (size_t)min.m128_f32[3];
  const size_t sMaxX = (size_t)max.m128_f32[3];
  const size_t sMinY = (size_t)min.m128_f32[2];
  const size_t sMaxY = (size_t)max.m128_f32[2];
  const size_t sMaxWidth = (size_t)maxWidth;

  __m128 xValues = _mm_set_ps(v2[0], v3[0], v1[0], 0.f);
  __m128 yValues = _mm_set_ps(v2[1], v3[1], v1[1], 0.f);

  // Factor out some of the BarycentricArea equation that's constant for each vertex.
  __m128 factors0 = _mm_sub_ps(_mm_set_ps(v3[0], v1[0], v2[0], 0.f), xValues);
  __m128 factors1 = _mm_sub_ps(_mm_set_ps(v3[1], v1[1], v2[1], 0.f), yValues);

  __m128 p1 = _mm_sub_ps(_mm_set_ps1(min.m128_f32[2]), yValues);

  for (size_t h = sMinY; h < sMaxY; ++h) {
    __m128 p0 = _mm_sub_ps(_mm_set_ps1(min.m128_f32[3]), xValues);

    __m128 p1Factor = _mm_mul_ps(p1, factors0);

    p1 = _mm_add_ps(p1, _mm_set_ps1(1.f));

    uint32* pixels = reinterpret_cast<uint32*>(framebuffer) + (sMinX + (h * sMaxWidth));
    float* pixelDepth = depthBuffer + (sMinX + (h * sMaxWidth));
    for (size_t w = sMinX; w < sMaxX; ++w, ++pixels, ++pixelDepth) {
      __m128 weights = _mm_sub_ps(_mm_mul_ps(p0, factors1), p1Factor);

      p0 = _mm_add_ps(p0, _mm_set_ps1(1.f));

      if (weights.m128_f32[3] > 0.f && weights.m128_f32[2] > 0.f && weights.m128_f32[1] > 0.f) {
        __m128 weightedVerts = _mm_mul_ps(weights, invVert);

        float pixelZ = weightedVerts.m128_f32[3] + weightedVerts.m128_f32[2] + weightedVerts.m128_f32[1];
        if ((*pixelDepth) > pixelZ) {
          *pixelDepth = pixelZ;

          __m128 denominator = _mm_set_ps1(weightedVerts.m128_f32[3] + weightedVerts.m128_f32[2] + weightedVerts.m128_f32[1]);

          __m128 weightedAttr0 = _mm_div_ps(_mm_mul_ps(weights, invAttr0), denominator);
          __m128 weightedAttr1 = _mm_div_ps(_mm_mul_ps(weights, invAttr1), denominator);
          __m128 weightedAttr2 = _mm_div_ps(_mm_mul_ps(weights, invAttr2), denominator);

          __m128 scaleFactor = _mm_set_ps1(255.f);
          weightedAttr0 = _mm_mul_ps(weightedAttr0, scaleFactor);
          weightedAttr1 = _mm_mul_ps(weightedAttr1, scaleFactor);
          weightedAttr2 = _mm_mul_ps(weightedAttr2, scaleFactor);

          const float r = weightedAttr0.m128_f32[3] + weightedAttr0.m128_f32[2] + weightedAttr0.m128_f32[1];
          const float g = weightedAttr1.m128_f32[3] + weightedAttr1.m128_f32[2] + weightedAttr1.m128_f32[1];
          const float b = weightedAttr2.m128_f32[3] + weightedAttr2.m128_f32[2] + weightedAttr2.m128_f32[1];
          *pixels = ColorFromRGB((uint8)r, (uint8)g, (uint8)b);
        }
      }
    }
  }
}

void Unaligned_FlatShadeUVs(const float* v1, const float* v2, const float* v3, const float maxWidth, const float maxHeight, uint8* framebuffer, float* depthBuffer, const Texture* texture) {
  __m128 min = _mm_set_ps(v1[0], v1[1], 0.f, 0.f);
  __m128 min1 = _mm_set_ps(v2[0], v2[1], 0.f, 0.f);
  __m128 min2 = _mm_set_ps(v3[0], v3[1], 0.f, 0.f);

  __m128 max = _mm_set_ps(v1[0], v1[1], 0.f, 0.f);
  __m128 max1 = _mm_set_ps(v2[0], v2[1], 0.f, 0.f);
  __m128 max2 = _mm_set_ps(v3[0], v3[1], 0.f, 0.f);

  min = _mm_min_ps(_mm_min_ps(min, min1), min2);
  max = _mm_max_ps(_mm_max_ps(max, max1), max2);

  min = _mm_floor_ps(min);
  max = _mm_ceil_ps(max);

  min = _mm_max_ps(min, _mm_set_ps1(0.f));
  min = _mm_min_ps(min, _mm_set_ps(maxWidth, maxHeight, 0.f, 0.f));
  max = _mm_max_ps(max, _mm_set_ps1(0.f));
  max = _mm_min_ps(max, _mm_set_ps(maxWidth, maxHeight, 0.f, 0.f));

  __m128 invArea = _mm_set_ps1(1.f / ((v3[0] - v1[0]) * (v2[1] - v1[1]) - ((v3[1] - v1[1]) * (v2[0] - v1[0]))));

  __m128 invVert = _mm_mul_ps(_mm_set_ps(v1[3], v2[3], v3[3], 0.f), invArea);
  __m128 invAttr0 = _mm_mul_ps(_mm_set_ps(v1[4], v2[4], v3[4], 0.f), invArea);
  __m128 invAttr1 = _mm_mul_ps(_mm_set_ps(v1[5], v2[5], v3[5], 0.f), invArea);

  const size_t sMinX = (size_t)min.m128_f32[3];
  const size_t sMaxX = (size_t)max.m128_f32[3];
  const size_t sMinY = (size_t)min.m128_f32[2];
  const size_t sMaxY = (size_t)max.m128_f32[2];
  const size_t sMaxWidth = (size_t)maxWidth;

  __m128 xValues = _mm_set_ps(v2[0], v3[0], v1[0], 0.f);
  __m128 yValues = _mm_set_ps(v2[1], v3[1], v1[1], 0.f);

  // Factor out some of the BarycentricArea equation that's constant for each vertex.
  __m128 factors0 = _mm_sub_ps(_mm_set_ps(v3[0], v1[0], v2[0], 0.f), xValues);
  __m128 factors1 = _mm_sub_ps(_mm_set_ps(v3[1], v1[1], v2[1], 0.f), yValues);

  __m128 p1 = _mm_sub_ps(_mm_set_ps1(min.m128_f32[2]), yValues);

  for (size_t h = sMinY; h < sMaxY; ++h) {
    __m128 p0 = _mm_sub_ps(_mm_set_ps1(min.m128_f32[3]), xValues);

    __m128 p1Factor = _mm_mul_ps(p1, factors0);

    p1 = _mm_add_ps(p1, _mm_set_ps1(1.f));

    uint32* pixels = reinterpret_cast<uint32*>(framebuffer) + (sMinX + (h * sMaxWidth));
    float* pixelDepth = depthBuffer + (sMinX + (h * sMaxWidth));
    for (size_t w = sMinX; w < sMaxX; ++w, ++pixels, ++pixelDepth) {
      __m128 weights = _mm_sub_ps(_mm_mul_ps(p0, factors1), p1Factor);

      p0 = _mm_add_ps(p0, _mm_set_ps1(1.f));

      if (weights.m128_f32[3] > 0.f && weights.m128_f32[2] > 0.f && weights.m128_f32[1] > 0.f) {
        __m128 weightedVerts = _mm_mul_ps(weights, invVert);

        float pixelZ = weightedVerts.m128_f32[3] + weightedVerts.m128_f32[2] + weightedVerts.m128_f32[1];
        if ((*pixelDepth) > pixelZ) {
          *pixelDepth = pixelZ;

          __m128 denominator = _mm_set_ps1(weightedVerts.m128_f32[3] + weightedVerts.m128_f32[2] + weightedVerts.m128_f32[1]);

          __m128 weightedAttr0 = _mm_div_ps(_mm_mul_ps(weights, invAttr0), denominator);
          __m128 weightedAttr1 = _mm_div_ps(_mm_mul_ps(weights, invAttr1), denominator);

          const float u = weightedAttr0.m128_f32[3] + weightedAttr0.m128_f32[2] + weightedAttr0.m128_f32[1];
          const float v = weightedAttr1.m128_f32[3] + weightedAttr1.m128_f32[2] + weightedAttr1.m128_f32[1];

          *pixels = texture->Sample(u, v);
        }
      }
    }
  }
}

}

#endif

#endif
