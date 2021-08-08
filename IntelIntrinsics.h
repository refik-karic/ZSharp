#pragma once

#include <cstdlib>

namespace ZSharp {

void avx512memsetaligned(void* __restrict dest, void* const __restrict value, const std::size_t numBytes);

}
