#pragma once

#include "ZBaseTypes.h"

namespace ZSharp {

class FrontEnd final {
  public:

  FrontEnd();

  ~FrontEnd();

  void Load();

  void Unload();

  void Tick();

  bool IsVisible() const;

  private:
  bool mVisible = false;
};

}
