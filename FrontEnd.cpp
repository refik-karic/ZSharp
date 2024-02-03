#include "FrontEnd.h"

namespace ZSharp {

FrontEnd::FrontEnd() {
}

FrontEnd::~FrontEnd() {
}

void FrontEnd::Load() {
  mVisible = true;
}

void FrontEnd::Unload() {
  mVisible = false;
}

void FrontEnd::Tick() {
}

bool FrontEnd::IsVisible() const {
  return mVisible;
}

}
