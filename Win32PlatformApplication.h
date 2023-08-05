#pragma once

#ifdef PLATFORM_WINDOWS

#include "Win32PlatformHeaders.h"

#include "Delegate.h"
#include "GameInstance.h"
#include "ZBaseTypes.h"

class Win32PlatformApplication {
  public:
  
  static Win32PlatformApplication& GetInstance();

  static LRESULT CALLBACK MessageLoop(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  int Run(HINSTANCE instance);

  static ZSharp::BroadcastDelegate<size_t, size_t> OnWindowSizeChangedDelegate;

  private:
  HINSTANCE mInstance = nullptr;
  HWND mWindowHandle = nullptr;
  HANDLE mHighPrecisionTimer = nullptr;
  bool mPaused = false;
  BITMAPINFO mBitmapInfo;

  ZSharp::GameInstance mGameInstance;

  Win32PlatformApplication();

  ~Win32PlatformApplication();

  HWND SetupWindow();

  // Win32 message loop handlers.

  void OnCreate(HWND initialHandle);

  static void OnTimerThunk(LPVOID optionalArg, DWORD timerLowVal, DWORD timerHighValue);

  void OnTimer();

  void OnPaint();

  void OnLButtonDown(ZSharp::int32 x, ZSharp::int32 y);

  void OnLButtonUp();

  void OnMouseMove(ZSharp::int32 x, ZSharp::int32 y);

  void OnKeyDown(ZSharp::uint8 key);

  void OnKeyUp(ZSharp::uint8 key);

  void OnPreWindowSizeChanged(LPMINMAXINFO info);

  void OnClose();

  void OnDestroy();

  // Everything else not related to message loop.

  void UpdateFrame(const ZSharp::uint8* data);

  void SplatTexture(const ZSharp::uint8* data, size_t width, size_t height, size_t bitsPerPixel);

  void UpdateAudio();
};

#endif
