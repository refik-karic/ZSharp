#pragma once

#include "ZBaseTypes.h"
#include "Delegate.h"

namespace ZSharp {

enum class AppCursor {
  Arrow,
  Hand
};

BroadcastDelegate<size_t, size_t>& OnWindowSizeChangedDelegate();

void AppChangeCursor(AppCursor cursor);

class PlatformApplication {
  public:

  virtual void ApplyCursor(AppCursor cursor) = 0;

  protected:
  AppCursor mCurrentCursor = AppCursor::Arrow;
};

}
