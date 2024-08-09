#pragma once

#ifdef PLATFORM_WINDOWS

#include "Win32PlatformHeaders.h"

#include "Delegate.h"
#include "GameInstance.h"
#include "ZBaseTypes.h"
#include "PlatformApplication.h"

class Win32PlatformApplication : public ZSharp::PlatformApplication {
  public:
  
  static Win32PlatformApplication& Get();

  ~Win32PlatformApplication();

  static LRESULT CALLBACK MessageLoop(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  int Run(HINSTANCE instance);

  virtual void ApplyCursor(ZSharp::AppCursor cursor) override;

  virtual void Shutdown() override;

  private:
  HINSTANCE mInstance = nullptr;
  HWND mWindowHandle = nullptr;
  HANDLE mHighPrecisionTimer = nullptr;
  bool mPaused = false;
  BITMAPINFO mBitmapInfo;

  HCURSOR mPointCursor = nullptr;
  HCURSOR mHandCursor = nullptr;

  ZSharp::GameInstance mGameInstance;

  Win32PlatformApplication();

  void ReadCommandLine();

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

  void PauseTimer();

  void StartTimer(ZSharp::int64 relativeNanoseconds);

  static bool IsSpecialKey(ZSharp::int32 key);
};

#endif
