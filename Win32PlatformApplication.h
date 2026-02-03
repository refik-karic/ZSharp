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
    bool mPaused : 1;
    bool mHidden : 1;
    bool mRunning : 1;
    bool mWaitingForPaint : 1;
  } mFlags;
  HWND mWindowHandle;
  HDC mWindowContext = nullptr;
  BITMAPINFO* mBitmapInfo = nullptr;
  BYTE* mKeyboard = nullptr;

  HCURSOR mPointCursor = nullptr;
  HCURSOR mHandCursor = nullptr;

  ZSharp::GameInstance* mGameInstance = nullptr;

  void ReadCommandLine();

  HWND SetupWindow(HINSTANCE instance);

  // Win32 message loop handlers.

  void OnCreate(HWND window);

  void Tick();

  void OnPaint();

  void OnLButtonDown(ZSharp::int32 x, ZSharp::int32 y);

  void OnLButtonUp();

  void OnMouseMove(ZSharp::int32 x, ZSharp::int32 y);

  void OnKeyDown(ZSharp::uint8 key);

  void OnKeyUp(ZSharp::uint8 key);

  void OnWindowResize();

  void OnPreWindowSizeChanged(LPMINMAXINFO info);

  void OnWindowVisibility(WPARAM param);

  void OnClose();

  void OnDestroy();

  // Everything else not related to message loop.

  void UpdateFrame(const ZSharp::uint8* data);

  void SplatTexture(const ZSharp::uint8* data, size_t width, size_t height, size_t bitsPerPixel);

  void UpdateAudio();

  void UpdateWindowSize(const RECT* rect);

  // We want to handle "special" keys different than input keys, so we don't call TranslateMsg in our MessageLoop.
  void TranslateKey(WPARAM key, WORD scanCode, bool isDown);

  static bool IsSpecialKey(ZSharp::int32 key);
};

extern Win32PlatformApplication* GlobalApplication;

#endif
