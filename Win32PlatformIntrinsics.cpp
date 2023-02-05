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

void Aligned_Mat4x4Transform(const Mat4x4& matrix, float* data, size_t stride, size_t length) {
  if (PlatformSupportsSIMDLanes(SIMDLaneWidth::Four)) {
    __m128 v0 = _mm_load_ps(*(matrix[0]));
    __m128 v1 = _mm_load_ps(*(matrix[1]));
    __m128 v2 = _mm_load_ps(*(matrix[2]));
    __m128 v3 = _mm_load_ps(*(matrix[3]));

    for (size_t i = 0; i < length; i += stride) {
      __m128 vec = _mm_load_ps(data + i);

      __m128 x = _mm_mul_ps(vec, v0);
      __m128 y = _mm_mul_ps(vec, v1);
      __m128 z = _mm_mul_ps(vec, v2);
      __m128 w = _mm_mul_ps(vec, v3);

      x = _mm_hadd_ps(x, x);
      x = _mm_hadd_ps(x, x);

      y = _mm_hadd_ps(y, y);
      y = _mm_hadd_ps(y, y);

      z = _mm_hadd_ps(z, z);
      z = _mm_hadd_ps(z, z);

      w = _mm_hadd_ps(w, w);
      w = _mm_hadd_ps(w, w);

      vec.m128_f32[0] = _mm_cvtss_f32(x);
      vec.m128_f32[1] = _mm_cvtss_f32(y);
      vec.m128_f32[2] = _mm_cvtss_f32(z);
      vec.m128_f32[3] = _mm_cvtss_f32(w);

      _mm_store_ps(data + i, vec);
    }
  }
  else {
    for (size_t i = 0; i < length; i += stride) {
      Vec4& vec = *(reinterpret_cast<Vec4*>(data + i));

      float xyzw[4];
      xyzw[0] = matrix[0] * vec;
      xyzw[1] = matrix[1] * vec;
      xyzw[2] = matrix[2] * vec;
      xyzw[3] = matrix[3] * vec;

      memcpy(data + i, xyzw, sizeof(xyzw));
    }
  }
}

void Aligned_Vec4Homogenize(float* data, size_t stride, size_t length) {
  if (PlatformSupportsSIMDLanes(SIMDLaneWidth::Four)) {
    for (size_t i = 0; i < length; i += stride) {
      float* nextVec = data + i;
      _mm_store_ps(nextVec, _mm_div_ps(_mm_load_ps(nextVec), _mm_set_ps1(nextVec[3])));
    }
  }
  else {
    for (size_t i = 0; i < length; i += stride) {
      float* vec = data + i;
      const float invDivisor = 1.f / vec[3];
      vec[0] /= invDivisor;
      vec[1] /= invDivisor;
      vec[2] /= invDivisor;
      vec[3] /= invDivisor;
    }
  }
}

}

#endif

#endif
