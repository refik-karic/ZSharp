#pragma once

#if defined(_WIN64)

#include "Win32PlatformHeaders.h"

#include "GameInstance.h"
#include "ZBaseTypes.h"

class Win32PlatformApplication {
  public:
  
  static Win32PlatformApplication& GetInstance();

  static LRESULT CALLBACK MessageLoop(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  int Run(HINSTANCE instance);

  private:
  HINSTANCE mInstance = nullptr;
  HWND mWindowHandle = nullptr;
  UINT_PTR mWindowsFrameTimer = 0;
  BITMAPINFO mBitmapInfo;

  ZSharp::GameInstance mGameInstance;

  Win32PlatformApplication();

  ~Win32PlatformApplication();

  HWND SetupWindow();

  // Win32 message loop handlers.

  void OnCreate(HWND initialHandle);

  void OnTimer();

  void OnPaint();

  void OnLButtonDown(ZSharp::int32 x, ZSharp::int32 y);

  void OnLButtonUp();

  void OnMouseMove(ZSharp::int32 x, ZSharp::int32 y);

  void OnKeyDown(ZSharp::uint8 key);

  void OnKeyUp(ZSharp::uint8 key);

  void OnClose();

  void OnDestroy();

  // Everything else not related to message loop.

  void UpdateFrame(const ZSharp::uint8* data);
};

#endif
