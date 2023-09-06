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

}

#endif

#endif
