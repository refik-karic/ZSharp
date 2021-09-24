#pragma once

#include <cstdlib>

namespace ZSharp {

void aligned_sse128addfloats(const float* a, const float* b, float* dest);

void aligned_sse128subfloats(const float* a, const float* b, float* dest);

void aligned_sse128mulfloat(const float* a, const float b, float* dest);

float aligned_sse128mulfloatssum(const float* a, const float* b);

void aligned_avx512memset(void* __restrict dest, void* const __restrict value, const size_t numBytes);

void unaligned_sse128addfloats(const float* a, const float* b, float* dest);

void unaligned_sse128subfloats(const float* a, const float* b, float* dest);

void unaligned_sse128mulfloat(const float* a, const float b, float* dest);

float unaligned_sse128mulfloatssum(const float* a, const float* b);

}
