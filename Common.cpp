#include "Common.h"

#if PLATFORM_WINDOWS
#include <stdlib.h>
#endif

namespace ZSharp {
#if PLATFORM_WINDOWS

// TODO: This still doesn't generate bswap in Debug. See if there's a way to force it.
uint16 EndianSwap(uint16 value) {
  return _byteswap_ushort(value);
}

uint32 EndianSwap(uint32 value) {
  return _byteswap_ulong(value);
}

#else

uint16 EndianSwap(uint16 value) {
  uint16 result = (value & 0x00FF) << 8;
  result |= (value & 0xFF00) >> 8;
  return result;
}

uint32 EndianSwap(uint32 value) {
  uint32 result = (value & 0x000000FF) << 24;
  result |= (value & 0x0000FF00) << 8;
  result |= (value & 0x00FF0000) >> 8;
  result |= (value & 0xFF000000) >> 24;
  return result;
}

#endif

}
