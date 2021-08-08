#include "IntelIntrinsics.h"

#include <cstdint>
#include <cstddef>
#include <immintrin.h>

namespace ZSharp {

void avx512memsetaligned(void* __restrict dest, void* const __restrict value, const std::size_t numBytes) {
  std::size_t* nextDest = reinterpret_cast<std::size_t*>(dest);
  const std::size_t end = numBytes / sizeof(std::size_t);

  __m512i repData = _mm512_load_epi32(value);
  for (std::size_t i = 0; i < end; i += sizeof(std::size_t)) {
    _mm512_store_epi32(nextDest + i, repData);
  }
}

}
