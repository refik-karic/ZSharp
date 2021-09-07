#include "IntelIntrinsics.h"

#include <cstdint>
#include <cstddef>
#include <immintrin.h>

namespace ZSharp {
void sse128addufloats(const float* a, const float* b, float* dest) {
  _mm_storeu_ps(dest, _mm_add_ps(_mm_loadu_ps(a), _mm_loadu_ps(b)));
}

void sse128subufloats(const float* a, const float* b, float* dest) {
  _mm_storeu_ps(dest, _mm_sub_ps(_mm_loadu_ps(a), _mm_loadu_ps(b)));
}

void sse128mulufloat(const float* a, const float b, float* dest) {
  _mm_storeu_ps(dest, _mm_mul_ps(_mm_loadu_ps(a), _mm_set_ps1(b)));
}

float sse128mulufloatssum(const float* a, const float* b) {
  __m128 mulResult = _mm_mul_ps(_mm_loadu_ps(a), _mm_loadu_ps(b));
  mulResult = _mm_hadd_ps(mulResult, mulResult);
  mulResult = _mm_hadd_ps(mulResult, mulResult);
  return _mm_cvtss_f32(mulResult);
}

void avx512memsetaligned(void* __restrict dest, void* const __restrict value, const std::size_t numBytes) {
  std::size_t* nextDest = reinterpret_cast<std::size_t*>(dest);
  const std::size_t end = numBytes / sizeof(std::size_t);

  __m512i repData = _mm512_load_epi32(value);
  for (std::size_t i = 0; i < end; i += sizeof(std::size_t)) {
    _mm512_store_epi32(nextDest + i, repData);
  }
}

}
