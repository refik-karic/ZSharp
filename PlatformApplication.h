#pragma once

#include "ZBaseTypes.h"
#include "Delegate.h"

namespace ZSharp {

enum class AppCursor {
  Arrow,
  Hand
};

BroadcastDelegate<size_t, size_t>& OnWindowSizeChangedDelegate();

class PlatformApplication {
  public:

  virtual void ApplyCursor(AppCursor cursor) = 0;

  virtual void Shutdown() = 0;

  protected:
  AppCursor mCurrentCursor = AppCursor::Arrow;
};

void InitializeEnvironment();

PlatformApplication* GetApplication();

bool IsKeyPressed(uint8 key);

}
