#pragma once

#include "ZBaseTypes.h"
#include "ZString.h"

namespace ZSharp {

enum class SIMDMode {
  SSE2,
  SSE4,
  AVX2,
  AVX512
};

bool PlatformSupportsSIMDMode(SIMDMode mode);

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

}