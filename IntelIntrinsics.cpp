#include "IntelIntrinsics.h"

#include <cstdint>
#include <cstddef>
#include <immintrin.h>

namespace ZSharp {
void aligned_sse128addfloats(const float* a, const float* b, float* dest) {
  _mm_store_ps(dest, _mm_add_ps(_mm_load_ps(a), _mm_load_ps(b)));
}

void aligned_sse128subfloats(const float* a, const float* b, float* dest) {
  _mm_store_ps(dest, _mm_sub_ps(_mm_load_ps(a), _mm_load_ps(b)));
}

void aligned_sse128mulfloat(const float* a, const float b, float* dest) {
  _mm_store_ps(dest, _mm_mul_ps(_mm_load_ps(a), _mm_set_ps1(b)));
}

float aligned_sse128mulfloatssum(const float* a, const float* b) {
  __m128 mulResult = _mm_mul_ps(_mm_load_ps(a), _mm_load_ps(b));
  mulResult = _mm_hadd_ps(mulResult, mulResult);
  mulResult = _mm_hadd_ps(mulResult, mulResult);
  return _mm_cvtss_f32(mulResult);
}

void aligned_avx512memset(void* __restrict dest, void* const __restrict value, const size_t numBytes) {
  size_t* nextDest = reinterpret_cast<size_t*>(dest);
  const size_t end = numBytes / sizeof(size_t);

  __m512i repData = _mm512_load_epi32(value);
  for (size_t i = 0; i < end; i += sizeof(size_t)) {
    _mm512_store_epi32(nextDest + i, repData);
  }
}

void unaligned_sse128addfloats(const float* a, const float* b, float* dest) {
  _mm_storeu_ps(dest, _mm_add_ps(_mm_loadu_ps(a), _mm_loadu_ps(b)));
}

void unaligned_sse128subfloats(const float* a, const float* b, float* dest) {
  _mm_storeu_ps(dest, _mm_sub_ps(_mm_loadu_ps(a), _mm_loadu_ps(b)));
}

void unaligned_sse128mulfloat(const float* a, const float b, float* dest) {
  _mm_storeu_ps(dest, _mm_mul_ps(_mm_loadu_ps(a), _mm_set_ps1(b)));
}

float unaligned_sse128mulfloatssum(const float* a, const float* b) {
  __m128 mulResult = _mm_mul_ps(_mm_loadu_ps(a), _mm_loadu_ps(b));
  mulResult = _mm_hadd_ps(mulResult, mulResult);
  mulResult = _mm_hadd_ps(mulResult, mulResult);
  return _mm_cvtss_f32(mulResult);
}

}
