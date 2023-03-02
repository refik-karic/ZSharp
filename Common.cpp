#include "Common.h"

namespace ZSharp {

uint32 EndianSwap(uint32 value) {
  uint32 result = (value & 0x000000FF) << 24;
  result |= (value & 0x0000FF00) << 8;
  result |= (value & 0x00FF0000) >> 8;
  result |= (value & 0xFF000000) >> 24;
  return result;
}

}
