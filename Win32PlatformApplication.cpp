#ifdef PLATFORM_WINDOWS

#include "Win32PlatformApplication.h"

#include "Common.h"
#include "InputManager.h"
#include "PlatformMemory.h"
#include "PNG.h"
#include "ScopedTimer.h"
#include "ZConfig.h"
#include "ZString.h"
#include "PlatformTime.h"
#include "CommandLineParser.h"

#include <synchapi.h>
#include <timeapi.h>
#include <processenv.h>
#include <shellapi.h>

#define DEBUG_TEXTURE 0

static ZSharp::WideString WindowClassName(L"SoftwareRendererWindowClass");
static ZSharp::WideString TimerName(L"MainLoop");
static constexpr LONG FRAMERATE_60_HZ_MS = 1000 / 60;
UINT MinTimerPeriod = 0;
DWORD WindowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

ZSharp::BroadcastDelegate<size_t, size_t> Win32PlatformApplication::OnWindowSizeChangedDelegate;

Win32PlatformApplication& Win32PlatformApplication::GetInstance() {
  static Win32PlatformApplication ZSharpApp;
  return ZSharpApp;
}

LRESULT Win32PlatformApplication::MessageLoop(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  Win32PlatformApplication& app = GetInstance();

  switch (uMsg) {
  case WM_CREATE:
    app.OnCreate(hwnd);
    return 0;
  case WM_PAINT:
    app.OnPaint();
    return 0;
  case WM_ERASEBKGND:
    return true;
  case WM_LBUTTONDOWN:
    app.OnLButtonDown(LOWORD(lParam), HIWORD(lParam));
    return 0;
  case WM_LBUTTONUP:
    app.OnLButtonUp();
    return 0;
  case WM_MOUSEMOVE:
    app.OnMouseMove(LOWORD(lParam), HIWORD(lParam));
    return 0;
  case WM_KEYDOWN:
    app.OnKeyDown(static_cast<ZSharp::uint8>(wParam));
    return 0;
  case WM_KEYUP:
    app.OnKeyUp(static_cast<ZSharp::uint8>(wParam));
    return 0;
  case WM_GETMINMAXINFO:
    app.OnPreWindowSizeChanged((LPMINMAXINFO)lParam);
    break;
  case WM_CLOSE:
    app.OnClose();
    break;
  case WM_DESTROY:
    app.OnDestroy();
    break;
  case WM_QUIT:
    return wParam;
  default:
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
  }

  return 0;
}

int Win32PlatformApplication::Run(HINSTANCE instance) {
  if (mWindowHandle == nullptr) {
    mInstance = instance;

    ReadCommandLine();

    mWindowHandle = SetupWindow();
    if (mWindowHandle == nullptr) {
      DWORD error = GetLastError();
      HRESULT result = HRESULT_FROM_WIN32(error);
      return result;
    }

    mGameInstance.Initialize();

    ShowWindow(mWindowHandle, SW_SHOW);
    for (MSG msg; mWindowHandle != nullptr;) {
      while (PeekMessageW(&msg, mWindowHandle, 0, 0, 0)) {
        if (GetMessageW(&msg, mWindowHandle, 0, 0)) {
          TranslateMessage(&msg);
          DispatchMessageW(&msg);
        }
      }
      
      UpdateAudio();

      SleepEx(MinTimerPeriod, true);
    }

    UnregisterClassW(WindowClassName.Str(), mInstance);
    return 0;
  }
  else {
    return -1;
  }
}

Win32PlatformApplication::Win32PlatformApplication() {
  ZeroMemory(&mBitmapInfo, sizeof(BITMAPINFO));
  mBitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFO);
  mBitmapInfo.bmiHeader.biWidth = 0;
  mBitmapInfo.bmiHeader.biHeight = 0;
  mBitmapInfo.bmiHeader.biPlanes = 1;
  mBitmapInfo.bmiHeader.biBitCount = 32;
  mBitmapInfo.bmiHeader.biCompression = BI_RGB;
  mBitmapInfo.bmiHeader.biSizeImage = 0;
  mBitmapInfo.bmiHeader.biXPelsPerMeter = 0;
  mBitmapInfo.bmiHeader.biYPelsPerMeter = 0;
  mBitmapInfo.bmiHeader.biClrUsed = 0;
  mBitmapInfo.bmiHeader.biClrImportant = 0;
}

Win32PlatformApplication::~Win32PlatformApplication() {
}

void Win32PlatformApplication::ReadCommandLine() {
  LPWSTR cmdLine = GetCommandLineW();
  ZSharp::int32 argC = 0;
  LPWSTR* argV = CommandLineToArgvW(cmdLine, &argC);

  ZSharp::Array<ZSharp::CLICommand> commands;

  ZSharp::Array<ZSharp::String> globalOptions;
  globalOptions.EmplaceBack("fullscreen");

  ZSharp::CLIParser cliParser(commands, globalOptions);
  ZSharp::int32 result = cliParser.Evaluate(argC, (const wchar_t**)argV, true); 
  
  LocalFree(argV);

  if(result != 0) {
    return;
  }

  // TODO: It might be a good idea to break this out into another file since it could be used elsewhere.
  if (cliParser.WasPassed("fullscreen")) {
    WindowStyle |= WS_MAXIMIZE;
  }
}

HWND Win32PlatformApplication::SetupWindow() {
  WNDCLASSEXW wc{
    sizeof(WNDCLASSEXW),
    CS_HREDRAW | CS_VREDRAW,
    &Win32PlatformApplication::MessageLoop,
    0,
    0,
    mInstance,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    WindowClassName.Str(),
    nullptr
  };

  if (!RegisterClassExW(&wc)) {
    return nullptr;
  }

  const ZSharp::ZConfig& config = ZSharp::ZConfig::GetInstance();

  DWORD windowStyle = WS_BORDER | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_THICKFRAME;
  RECT clientRect{ 0L, 0L, static_cast<long>(config.GetViewportWidth().Value()), static_cast<long>(config.GetViewportHeight().Value()) };
  AdjustWindowRectEx(&clientRect, windowStyle, false, WS_EX_OVERLAPPEDWINDOW);

  return CreateWindowExW(
    WS_EX_OVERLAPPEDWINDOW,
    WindowClassName.Str(),
    config.GetWindowTitle().ToWide().Str(),
    WindowStyle,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    clientRect.right - clientRect.left,
    clientRect.bottom - clientRect.top,
    nullptr,
    nullptr,
    mInstance,
    nullptr
  );
}

void Win32PlatformApplication::OnCreate(HWND initialHandle) {
  TIMECAPS timecaps = {};
  if (timeGetDevCaps(&timecaps, sizeof(timecaps)) != MMSYSERR_NOERROR) {
    DestroyWindow(initialHandle);
    return;
  }

  MinTimerPeriod = timecaps.wPeriodMin;

  if (timeBeginPeriod(MinTimerPeriod) != MMSYSERR_NOERROR) {
    DestroyWindow(initialHandle);
    return;
  }


  mHighPrecisionTimer = CreateWaitableTimerW(NULL, false, TimerName.Str());

  if (mHighPrecisionTimer == INVALID_HANDLE_VALUE) {
    DestroyWindow(initialHandle);
    return;
  }

  StartTimer((ZSharp::int64)1);
}

void Win32PlatformApplication::OnTimerThunk(LPVOID optionalArg, DWORD timerLowVal, DWORD timerHighValue) {
  (void)timerLowVal;
  (void)timerHighValue;

  Win32PlatformApplication* app = (Win32PlatformApplication*)optionalArg;
  if (!app->mPaused) {
    app->OnTimer();
  }
}

void Win32PlatformApplication::OnTimer() {
  size_t frameDeltaTime = ZSharp::PlatformHighResClock();

  PauseTimer();

  RECT activeWindowSize;
  if (GetClientRect(mWindowHandle, &activeWindowSize)) {
    ZSharp::ZConfig& config = ZSharp::ZConfig::GetInstance();

    bool dirtySize = false;

    if (activeWindowSize.right != config.GetViewportWidth().Value()) {
      config.SetViewportWidth(activeWindowSize.right);
      dirtySize = true;
    }

    if (activeWindowSize.bottom != config.GetViewportHeight().Value()) {
      config.SetViewportHeight(activeWindowSize.bottom);
      dirtySize = true;
    }

    mBitmapInfo.bmiHeader.biWidth = activeWindowSize.right;
    mBitmapInfo.bmiHeader.biHeight = -activeWindowSize.bottom;

    if (dirtySize) {
      OnWindowSizeChangedDelegate.Broadcast(activeWindowSize.right, activeWindowSize.bottom);
    }
  }

  mGameInstance.Tick();

  InvalidateRect(mWindowHandle, &activeWindowSize, false);

  // Sleep if we have some time left in the frame, otherwise start again immediately.
  frameDeltaTime = ZSharp::PlatformHighResClockDelta(frameDeltaTime, ZSharp::ClockUnits::Milliseconds);
  if (frameDeltaTime >= FRAMERATE_60_HZ_MS) {
    frameDeltaTime = 1;
  }
  else {
    frameDeltaTime = FRAMERATE_60_HZ_MS - frameDeltaTime;
    frameDeltaTime = (frameDeltaTime * 10000);
  }

  StartTimer((ZSharp::int64)frameDeltaTime);
}

void Win32PlatformApplication::OnPaint() {
#if DEBUG_TEXTURE
  ZSharp::NamedScopedTimer(SplatTexture);

  ZSharp::FileString texturePath(ZSharp::PlatformGetUserDesktopPath());
  texturePath.SetFilename("wall_256.png");

  ZSharp::PNG png(texturePath);
  ZSharp::uint8* pngData = png.Decompress(ZSharp::ChannelOrder::BGR);
  ZSharp::size_t width = png.GetWidth();
  ZSharp::size_t height = png.GetHeight();
  ZSharp::size_t bitsPerPixel = png.GetBitsPerPixel();

  SplatTexture(pngData, width, height, bitsPerPixel);
#if 0
  if (pngData != nullptr) {
    ZSharp::PlatformFree(pngData);
  }
#endif
#else
  UpdateFrame(mGameInstance.GetCurrentFrame());
#endif
}

void Win32PlatformApplication::OnLButtonDown(ZSharp::int32 x, ZSharp::int32 y) {
  ZSharp::InputManager& inputManager = ZSharp::InputManager::GetInstance();
  inputManager.UpdateMousePosition(x, y);
  inputManager.UpdateMouseState(true);
}

void Win32PlatformApplication::OnLButtonUp() {
  ZSharp::InputManager& inputManager = ZSharp::InputManager::GetInstance();
  inputManager.ResetMouse();
}

void Win32PlatformApplication::OnMouseMove(ZSharp::int32 x, ZSharp::int32 y) {
  ZSharp::InputManager& inputManager = ZSharp::InputManager::GetInstance();
  inputManager.UpdateMousePosition(x, y);
}

void Win32PlatformApplication::OnKeyDown(ZSharp::uint8 key) {
  switch (key) {
  case VK_SPACE:
    mPaused = !mPaused;
    break;
  case VK_ESCAPE:
    DestroyWindow(mWindowHandle);
    break;
  default:
  {
    ZSharp::InputManager& inputManager = ZSharp::InputManager::GetInstance();
    inputManager.Update(key, ZSharp::InputManager::KeyState::Down);
  }
  break;
  }
}

void Win32PlatformApplication::OnKeyUp(ZSharp::uint8 key) {
  ZSharp::InputManager& inputManager = ZSharp::InputManager::GetInstance();
  inputManager.Update(key, ZSharp::InputManager::KeyState::Up);
}

void Win32PlatformApplication::OnPreWindowSizeChanged(LPMINMAXINFO info) {
  const ZSharp::ZConfig& config = ZSharp::ZConfig::GetInstance();

  const ZSharp::GameSetting<size_t> width = config.GetViewportWidth();
  const ZSharp::GameSetting<size_t> height = config.GetViewportHeight();

  ZSharp::Clamp(info->ptMinTrackSize.x, (LONG)width.Min(), (LONG)width.Max());
  ZSharp::Clamp(info->ptMaxTrackSize.x, (LONG)width.Min(), (LONG)width.Max());
  ZSharp::Clamp(info->ptMinTrackSize.y, (LONG)height.Min(), (LONG)height.Max());
  ZSharp::Clamp(info->ptMaxTrackSize.y, (LONG)height.Min(), (LONG)height.Max());
}

void Win32PlatformApplication::OnClose() {
  DestroyWindow(mWindowHandle);
}

void Win32PlatformApplication::OnDestroy() {
  if (mHighPrecisionTimer != INVALID_HANDLE_VALUE) {
    PauseTimer();
    CloseHandle(mHighPrecisionTimer);
  }

  timeEndPeriod(MinTimerPeriod);

  mWindowHandle = nullptr;

  PostQuitMessage(0);
}

void Win32PlatformApplication::UpdateFrame(const ZSharp::uint8* data) {
  ZSharp::NamedScopedTimer(BlitFrame);

  PAINTSTRUCT ps;
  HDC hdc = BeginPaint(mWindowHandle, &ps);

  SetDIBitsToDevice(hdc, 
    0, 
    0, 
    mBitmapInfo.bmiHeader.biWidth, 
    -mBitmapInfo.bmiHeader.biHeight, 
    0, 
    0,
    0, 
    -mBitmapInfo.bmiHeader.biHeight,
    data, 
    &mBitmapInfo, 
    DIB_RGB_COLORS);

  EndPaint(mWindowHandle, &ps);
}

void Win32PlatformApplication::SplatTexture(const ZSharp::uint8* data, size_t width, size_t height, size_t bitsPerPixel) {
  if (data == nullptr) {
    return;
  }
  
  BITMAPINFO info;
  ZeroMemory(&info, sizeof(BITMAPINFO));
  info.bmiHeader.biSize = sizeof(BITMAPINFO);
  info.bmiHeader.biWidth = (LONG)width;
  info.bmiHeader.biHeight = -((LONG)height);
  info.bmiHeader.biPlanes = 1;
  info.bmiHeader.biBitCount = (WORD)bitsPerPixel;
  info.bmiHeader.biCompression = BI_RGB;
  info.bmiHeader.biSizeImage = 0;
  info.bmiHeader.biXPelsPerMeter = 0;
  info.bmiHeader.biYPelsPerMeter = 0;
  info.bmiHeader.biClrUsed = 0;
  info.bmiHeader.biClrImportant = 0;

  PAINTSTRUCT ps;
  HDC hdc = BeginPaint(mWindowHandle, &ps);

  SetDIBitsToDevice(hdc,
    0,
    0,
    info.bmiHeader.biWidth,
    -info.bmiHeader.biHeight,
    0,
    0,
    0,
    -info.bmiHeader.biHeight,
    data,
    &info,
    DIB_RGB_COLORS);

  EndPaint(mWindowHandle, &ps);
}

void Win32PlatformApplication::UpdateAudio() {
  mGameInstance.TickAudio();
}

void Win32PlatformApplication::PauseTimer() {
  CancelWaitableTimer(mHighPrecisionTimer);
}

void Win32PlatformApplication::StartTimer(ZSharp::int64 relativeNanoseconds) {
  LARGE_INTEGER dueTime = {};
  ZSharp::int64 dueTimeLarge = -1 * relativeNanoseconds;
  dueTime.LowPart = (DWORD)(dueTimeLarge & 0xFFFFFFFF);
  dueTime.HighPart = (LONG)(dueTimeLarge >> 32);

  SetWaitableTimer(mHighPrecisionTimer,
    &dueTime,
    FRAMERATE_60_HZ_MS - 2,
    &Win32PlatformApplication::OnTimerThunk,
    this,
    true);
}

#endif
