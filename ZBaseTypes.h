#pragma once

namespace ZSharp {
  typedef unsigned char uint8;
  typedef unsigned short uint16;
  typedef unsigned int uint32;
  typedef unsigned long long uint64;

  typedef char int8;
  typedef short int16;
  typedef int int32;
  typedef long long int64;

  constexpr uint8 min_uint8 = 0x0U;
  constexpr uint16 min_uint16 = 0x0U;
  constexpr uint32 min_uint32 = 0x0U;
  constexpr uint64 min_uint64 = 0x0ULL;

  constexpr uint8 max_uint8 = 0xFFU;
  constexpr uint16 max_uint16 = 0xFFFFU;
  constexpr uint32 max_uint32 = 0xFFFFFFFFU;
  constexpr uint64 max_uint64 = 0xFFFFFFFFFFFFFFFFULL;

  constexpr int8 min_int8 = -128;
  constexpr int16 min_int16 = -32768;

#ifdef __clang__
  constexpr int32 min_int32 = -2147483648;
  constexpr int64 min_int64 = -9223372036854775807LL - 1LL;
#else
  constexpr int32 min_int32 = -2147483648i32;
  constexpr int64 min_int64 = -9223372036854775808i64;
#endif

  constexpr int8 max_int8 = 0x7F;
  constexpr int16 max_int16 = 0x7FFF;
  constexpr int32 max_int32 = 0x7FFFFFFF;
  constexpr int64 max_int64 = 0x7FFFFFFFFFFFFFFF;

#ifdef _M_X64
  typedef uint64 size_t;
  typedef int64 ssize_t;

  constexpr int64 min_ssize_t = min_int64;
  constexpr int64 max_ssize_t = max_int64;
  constexpr size_t max_size_t = max_uint64;
#elif _M_X86
  typedef uint32 size_t;
  typedef int32 ssize_t;

  constexpr int32 min_ssize_t = min_int32;
  constexpr int632 max_ssize_t = max_int32;
  constexpr size_t max_size_t = max_uint32;
#endif

  constexpr size_t min_size_t = 0x0ULL;
}
