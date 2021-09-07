#pragma once

#include <cstdlib>

namespace ZSharp {

void sse128addufloats(const float* a, const float* b, float* dest);

void sse128subufloats(const float* a, const float* b, float* dest);

void sse128mulufloat(const float* a, const float b, float* dest);

float sse128mulufloatssum(const float* a, const float* b);

void avx512memsetaligned(void* __restrict dest, void* const __restrict value, const std::size_t numBytes);

}
