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

#ifdef _M_X64
  typedef uint64 size_t;
  typedef int64 ssize_t;
#elif _M_X86
  typedef uint32 size_t;
  typedef int32 ssize_t;
#endif
}
