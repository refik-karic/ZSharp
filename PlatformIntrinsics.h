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

void Aligned_Memset(void* __restrict dest, void* const __restrict value, const size_t numBytes);

void Unaligned_128Add(const float* a, const float* b, float* dest);

void Unaligned_128Sub(const float* a, const float* b, float* dest);

void Unaligned_128MulByValue(const float* a, const float b, float* dest);

float Unaligned_128MulSum(const float* a, const float* b);

}
