#pragma once

#include "ZBaseTypes.h"
#include "Array.h"

namespace ZSharp {

enum class ShadingModes {
  None,
  RGB,
  UV,
  Normals
};

struct ShadingMode {
  ShadingModes mode;
  size_t length;

  ShadingMode() : mode(ShadingModes::None), length(0) {};
  ShadingMode(ShadingModes inMode, size_t inLength) : mode(inMode), length(inLength) {};
  ShadingMode(const ShadingMode& rhs) : mode(rhs.mode), length(rhs.length) {};
};

typedef Array<ShadingMode> ShadingModeOrder;

}

