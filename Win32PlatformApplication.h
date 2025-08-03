#pragma once

#ifdef PLATFORM_WINDOWS

#include "Win32PlatformHeaders.h"

#include "Delegate.h"
#include "GameInstance.h"
#include "ZBaseTypes.h"
#include "PlatformApplication.h"

class Win32PlatformApplication : public ZSharp::PlatformApplication {
  public:
  
  Win32PlatformApplication();

  ~Win32PlatformApplication();

  static LRESULT CALLBACK MessageLoop(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  int Run(HINSTANCE instance);

  virtual void ApplyCursor(ZSharp::AppCursor cursor) override;

  virtual void Shutdown() override;

  private:
  struct {
    ZSharp::uint8 mPaused : 1;
    ZSharp::uint8 mHidden : 1;
    ZSharp::uint8 mRunning : 1;
  } mFlags;
  HDC mWindowContext = nullptr;
  BITMAPINFO* mBitmapInfo = nullptr;
  BYTE* mKeyboard = nullptr;

  HCURSOR mPointCursor = nullptr;
  HCURSOR mHandCursor = nullptr;

  ZSharp::GameInstance* mGameInstance = nullptr;

  void ReadCommandLine();

  HWND SetupWindow(HINSTANCE instance);

  // Win32 message loop handlers.

  void OnCreate(HWND initialHandle);

  DWORD Tick(HWND window);

  void OnPaint(HWND window);

  void OnLButtonDown(ZSharp::int32 x, ZSharp::int32 y);

  void OnLButtonUp();

  void OnMouseMove(ZSharp::int32 x, ZSharp::int32 y);

  void OnKeyDown(HWND window, ZSharp::uint8 key);

  void OnKeyUp(ZSharp::uint8 key);

  void OnWindowResize(HWND window);

  void OnPreWindowSizeChanged(LPMINMAXINFO info);

  void OnWindowVisibility(HWND window, WPARAM param);

  void OnClose(HWND window);

  void OnDestroy(HWND window);

  // Everything else not related to message loop.

  void UpdateFrame(HWND window, const ZSharp::uint8* data);

  void SplatTexture(HWND window, const ZSharp::uint8* data, size_t width, size_t height, size_t bitsPerPixel);

  void UpdateAudio();

  void UpdateWindowSize(const RECT* rect);

  // We want to handle "special" keys different than input keys, so we don't call TranslateMsg in our MessageLoop.
  void TranslateKey(HWND window, WPARAM key, WORD scanCode, bool isDown);

  static bool IsSpecialKey(ZSharp::int32 key);
};

extern Win32PlatformApplication* GlobalApplication;

#endif
