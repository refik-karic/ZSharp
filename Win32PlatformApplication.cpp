#ifdef PLATFORM_WINDOWS

#include "Win32PlatformApplication.h"

#include "Common.h"
#include "ConsoleVariable.h"
#include "InputManager.h"
#include "PlatformMemory.h"
#include "PNG.h"
#include "JPEG.h"
#include "ScopedTimer.h"
#include "ZConfig.h"
#include "ZString.h"
#include "PlatformTime.h"
#include "CommandLineParser.h"
#include "PlatformIntrinsics.h"

#include <synchapi.h>
#include <timeapi.h>
#include <processenv.h>
#include <shellapi.h>

#include <ctype.h>

#define DEBUG_TEXTURE_PNG 0
#define DEBUG_TEXTURE_JPG 0

static ZSharp::WideString WindowClassName(L"SoftwareRendererWindowClass");
static ZSharp::WideString TimerName(L"MainLoop");
UINT MinTimerPeriod = 0;
DWORD WindowStyle = WS_BORDER | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_THICKFRAME;

ZSharp::ConsoleVariable<bool> UncappedFPS("UncappedFPS", false);
ZSharp::ConsoleVariable<ZSharp::int32> LockedFPS("LockedFPS", 60);

namespace ZSharp {

BroadcastDelegate<size_t, size_t>& OnWindowSizeChangedDelegate() {
  static BroadcastDelegate<size_t, size_t> instance;
  return instance;
}

void InitializeEnvironment() {
  // Ignoring AVX512 for now.
  if (PlatformSupportsSIMDLanes(SIMDLaneWidth::Eight)) {
    RGBShaderImpl = &Unaligned_Shader_RGB_AVX;
    UVShaderImpl = &Unaligned_Shader_UV_AVX;
    CalculateAABBImpl = &Unaligned_AABB_AVX;
    DrawDebugTextImpl = &Unaligned_DrawDebugText_AVX;
    DepthBufferVisualizeImpl = &Aligned_DepthBufferVisualize_AVX;
    BlendBuffersImpl = &Unaligned_BlendBuffers_AVX;
    BilinearScaleImageImpl = &Unaligned_BilinearScaleImage_AVX;
    GenerateMipLevelImpl = &Unaligned_GenerateMipLevel_AVX;
  }
  else if (PlatformSupportsSIMDLanes(SIMDLaneWidth::Four)) {
    RGBShaderImpl = &Unaligned_Shader_RGB_SSE;
    UVShaderImpl = &Unaligned_Shader_UV_SSE;
    CalculateAABBImpl = &Unaligned_AABB_SSE;
    DrawDebugTextImpl = &Unaligned_DrawDebugText_SSE;
    DepthBufferVisualizeImpl = &Aligned_DepthBufferVisualize_SSE;
    BlendBuffersImpl = &Unaligned_BlendBuffers_SSE;
    BilinearScaleImageImpl = &Unaligned_BilinearScaleImage_SSE;
    GenerateMipLevelImpl = &Unaligned_GenerateMipLevel_SSE;
  }
  else {
    ZAssert(false);
  }
}

PlatformApplication* GetApplication() {
  Win32PlatformApplication& app = Win32PlatformApplication::Get();
  return &app;
}

bool IsKeyPressed(uint8 key) {
  HKL keyboard = GetKeyboardLayout(0);
  short virtualKey = VkKeyScanExA(key, keyboard);
  return GetAsyncKeyState(virtualKey);
}

}

Win32PlatformApplication& Win32PlatformApplication::Get() {
  static Win32PlatformApplication ZSharpApp;
  return ZSharpApp;
}

LRESULT Win32PlatformApplication::MessageLoop(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  Win32PlatformApplication& app = Win32PlatformApplication::Get();

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
  {
    if (IsSpecialKey((ZSharp::int32)wParam)) {
      app.OnKeyDown(static_cast<ZSharp::uint8>(wParam));
    }
    else {
      UINT scanCode = MapVirtualKeyExW((UINT)wParam, MAPVK_VK_TO_VSC, 0);
      BYTE buff[256] = {};
      WORD keys[2] = {};
      if (ToAscii((UINT)wParam, scanCode, buff, keys, 0)) {
        int firstKey = keys[0];
        if (isalpha(firstKey) && !isdigit(firstKey) && IsShiftPressed()) {
          firstKey = (WORD)toupper(firstKey);
        }

        app.OnKeyDown(static_cast<ZSharp::uint8>(firstKey));
      }
    }
  }
    return 0;
  case WM_KEYUP:
  {
    if (IsSpecialKey((ZSharp::int32)wParam)) {
      app.OnKeyUp(static_cast<ZSharp::uint8>(wParam));
    }
    else {
      UINT scanCode = MapVirtualKeyExW((UINT)wParam, MAPVK_VK_TO_VSC, 0);
      BYTE buff[256] = {};
      WORD keys[2] = {};
      if (ToAscii((UINT)wParam, scanCode, buff, keys, 0)) {
        int firstKey = keys[0];
        if (isalpha(firstKey) && !isdigit(firstKey) && IsShiftPressed()) {
          firstKey = (WORD)toupper(firstKey);
        }

        app.OnKeyUp(static_cast<ZSharp::uint8>(firstKey));
      }
    }
  }
    return 0;
  case WM_GETMINMAXINFO:
    app.OnPreWindowSizeChanged((LPMINMAXINFO)lParam);
    break;
  case WM_SIZE:
    app.OnWindowVisibility(wParam);
    break;
  case WM_SIZING:
    app.OnWindowResize((const RECT*)lParam);
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

    mGameInstance->Initialize(false);

    ShowWindow(mWindowHandle, SW_SHOW);
    for (MSG msg; mWindowHandle != nullptr;) {
      while (PeekMessageW(&msg, mWindowHandle, 0, 0, 0)) {
        if (GetMessageW(&msg, mWindowHandle, 0, 0)) {
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

void Win32PlatformApplication::ApplyCursor(ZSharp::AppCursor cursor) {
  mCurrentCursor = cursor;

  switch (mCurrentCursor) {
    case ZSharp::AppCursor::Arrow:
      SetCursor(mPointCursor);
      break;
    case ZSharp::AppCursor::Hand:
      SetCursor(mHandCursor);
      break;
  }
}

void Win32PlatformApplication::Shutdown() {
  OnDestroy();
}

Win32PlatformApplication::Win32PlatformApplication()
  : mBitmapInfo{new BITMAPINFO()}, mPointCursor(nullptr), mHandCursor(nullptr) {
  mBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFO);
  mBitmapInfo->bmiHeader.biWidth = 0;
  mBitmapInfo->bmiHeader.biHeight = 0;
  mBitmapInfo->bmiHeader.biPlanes = 1;
  mBitmapInfo->bmiHeader.biBitCount = 32;
  mBitmapInfo->bmiHeader.biCompression = BI_RGB;
  mBitmapInfo->bmiHeader.biSizeImage = 0;
  mBitmapInfo->bmiHeader.biXPelsPerMeter = 0;
  mBitmapInfo->bmiHeader.biYPelsPerMeter = 0;
  mBitmapInfo->bmiHeader.biClrUsed = 0;
  mBitmapInfo->bmiHeader.biClrImportant = 0;

  mGameInstance = new ZSharp::GameInstance();
}

Win32PlatformApplication::~Win32PlatformApplication() {
  if (mBitmapInfo != nullptr) {
    delete mBitmapInfo;
  }

  if (mGameInstance != nullptr) {
    delete mGameInstance;
  }
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

  const ZSharp::ZConfig& config = ZSharp::ZConfig::Get();

  RECT clientRect{ 0L, 0L, static_cast<long>(config.GetViewportWidth().Value()), static_cast<long>(config.GetViewportHeight().Value()) };
  AdjustWindowRectEx(&clientRect, WindowStyle, false, WS_EX_OVERLAPPEDWINDOW);

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
  TIMECAPS timecaps{};
  if (timeGetDevCaps(&timecaps, sizeof(timecaps)) != MMSYSERR_NOERROR) {
    DestroyWindow(initialHandle);
    return;
  }

  MinTimerPeriod = timecaps.wPeriodMin;

  if (timeBeginPeriod(MinTimerPeriod) != MMSYSERR_NOERROR) {
    DestroyWindow(initialHandle);
    return;
  }

  mPointCursor = LoadCursor(NULL, IDC_ARROW);
  mHandCursor = LoadCursor(NULL, IDC_HAND);

  mCurrentCursor = ZSharp::AppCursor::Arrow;

  mHighPrecisionTimer = CreateWaitableTimerW(NULL, false, TimerName.Str());

  if (mHighPrecisionTimer == INVALID_HANDLE_VALUE) {
    DestroyWindow(initialHandle);
    return;
  }

  // We need to broadcast the final window size to the game code before start ticking.
  RECT activeWindowSize{};
  if (GetClientRect(initialHandle, &activeWindowSize)) {
    UpdateWindowSize(activeWindowSize);
  }

  mWindowContext = GetDC(initialHandle);
  if (mWindowContext == nullptr) {
    DestroyWindow(initialHandle);
    return;
  }

  StartTimer((ZSharp::int64)1);
}

void Win32PlatformApplication::OnTimerThunk(LPVOID optionalArg, DWORD timerLowVal, DWORD timerHighValue) {
  (void)timerLowVal;
  (void)timerHighValue;

  Win32PlatformApplication* app = (Win32PlatformApplication*)optionalArg;
  if (!app->mPaused && !app->mHidden) {
    app->OnTimer();
  }
}

void Win32PlatformApplication::OnTimer() {
  size_t frameDeltaTime = ZSharp::PlatformHighResClock();

  PauseTimer();

  if (mCurrentCursor != ZSharp::AppCursor::Arrow) {
    ApplyCursor(ZSharp::AppCursor::Arrow);
  }

  mGameInstance->Tick();

  InvalidateRect(mWindowHandle, NULL, false);

  // Sleep if we have some time left in the frame, otherwise start again immediately.
  frameDeltaTime = ZSharp::PlatformHighResClockDeltaMs(frameDeltaTime);
  if (frameDeltaTime >= (1000 / (*LockedFPS))) {
    frameDeltaTime = 1;
  }
  else {
    if (*UncappedFPS) {
      frameDeltaTime = 1;
    }
    else {
      frameDeltaTime = (1000 / (*LockedFPS)) - frameDeltaTime;
      frameDeltaTime = (frameDeltaTime * 10000);
    }
  }

  StartTimer((ZSharp::int64)frameDeltaTime);
}

void Win32PlatformApplication::OnPaint() {
#if DEBUG_TEXTURE_PNG
  ZSharp::NamedScopedTimer(SplatTexture);

  ZSharp::FileString texturePath(ZSharp::PlatformGetUserDesktopPath());
  texturePath.SetFilename("wall_256.png");

  ZSharp::PNG png(texturePath);
  ZSharp::uint8* pngData = png.Decompress(ZSharp::ChannelOrder::BGR);
  ZSharp::size_t width = png.GetWidth();
  ZSharp::size_t height = png.GetHeight();
  ZSharp::size_t bitsPerPixel = png.GetBitsPerPixel();

  SplatTexture(pngData, width, height, bitsPerPixel);

  if (pngData != nullptr) {
    ZSharp::PlatformFree(pngData);
  }
#elif DEBUG_TEXTURE_JPG
  ZSharp::NamedScopedTimer(SplatTexture);

  ZSharp::FileString texturePath(ZSharp::PlatformGetUserDesktopPath());
  texturePath.SetFilename("diffuse.jpg");

  ZSharp::JPEG jpg(texturePath);
  ZSharp::uint8* jpgData = jpg.Decompress(ZSharp::ChannelOrderJPG::BGR);
  ZSharp::size_t width = jpg.GetWidth();
  ZSharp::size_t height = jpg.GetHeight();
  ZSharp::size_t bitsPerPixel = jpg.GetNumChannels() * 8;

  SplatTexture(jpgData, width, height, bitsPerPixel);

  if (jpgData != nullptr) {
    ZSharp::PlatformFree(jpgData);
  }
#else
  UpdateFrame(mGameInstance->GetCurrentFrame());

  mGameInstance->RunBackgroundJobs();
#endif
}

void Win32PlatformApplication::OnLButtonDown(ZSharp::int32 x, ZSharp::int32 y) {
  ZSharp::InputManager& inputManager = ZSharp::InputManager::Get();
  inputManager.UpdateMousePosition(x, y);
  inputManager.UpdateMouseState(true);
}

void Win32PlatformApplication::OnLButtonUp() {
  ZSharp::InputManager& inputManager = ZSharp::InputManager::Get();
  inputManager.UpdateMouseState(false);
}

void Win32PlatformApplication::OnMouseMove(ZSharp::int32 x, ZSharp::int32 y) {
  ZSharp::InputManager& inputManager = ZSharp::InputManager::Get();
  inputManager.UpdateMousePosition(x, y);
}

void Win32PlatformApplication::OnKeyDown(ZSharp::uint8 key) {
  switch (key) {
  case VK_SPACE:
  {
    if (!mGameInstance->IsDevConsoleOpen()) {
       mPaused = !mPaused;
    }
    else {
      ZSharp::InputManager& inputManager = ZSharp::InputManager::Get();
      inputManager.Update(key, ZSharp::InputManager::KeyState::Down);
    }
  }
    break;
  case VK_ESCAPE:
    DestroyWindow(mWindowHandle);
    break;
  case VK_UP:
  {
    ZSharp::InputManager& inputManager = ZSharp::InputManager::Get();
    inputManager.UpdateMiscKey(ZSharp::MiscKey::UP_ARROW, ZSharp::InputManager::KeyState::Down);
  }
    break;
  case VK_LEFT:
  {
    ZSharp::InputManager& inputManager = ZSharp::InputManager::Get();
    inputManager.UpdateMiscKey(ZSharp::MiscKey::LEFT_ARROW, ZSharp::InputManager::KeyState::Down);
  }
  break;
  case VK_RIGHT:
  {
    ZSharp::InputManager& inputManager = ZSharp::InputManager::Get();
    inputManager.UpdateMiscKey(ZSharp::MiscKey::RIGHT_ARROW, ZSharp::InputManager::KeyState::Down);
  }
  break;
  case VK_DOWN:
  {
    ZSharp::InputManager& inputManager = ZSharp::InputManager::Get();
    inputManager.UpdateMiscKey(ZSharp::MiscKey::DOWN_ARROW, ZSharp::InputManager::KeyState::Down);
  }
  break;
  case VK_RETURN:
  {
    ZSharp::InputManager& inputManager = ZSharp::InputManager::Get();
    inputManager.UpdateMiscKey(ZSharp::MiscKey::ENTER, ZSharp::InputManager::KeyState::Down);
  }
  break;
  case VK_BACK:
  {
    ZSharp::InputManager& inputManager = ZSharp::InputManager::Get();
    inputManager.UpdateMiscKey(ZSharp::MiscKey::BACKSPACE, ZSharp::InputManager::KeyState::Down);
  }
  break;
  default:
  {
    ZSharp::InputManager& inputManager = ZSharp::InputManager::Get();
    inputManager.Update(key, ZSharp::InputManager::KeyState::Down);
  }
  break;
  }
}

void Win32PlatformApplication::OnKeyUp(ZSharp::uint8 key) {
  switch (key) {
    case VK_UP:
    {
      ZSharp::InputManager& inputManager = ZSharp::InputManager::Get();
      inputManager.UpdateMiscKey(ZSharp::MiscKey::UP_ARROW, ZSharp::InputManager::KeyState::Up);
    }
    break;
    case VK_LEFT:
    {
      ZSharp::InputManager& inputManager = ZSharp::InputManager::Get();
      inputManager.UpdateMiscKey(ZSharp::MiscKey::LEFT_ARROW, ZSharp::InputManager::KeyState::Up);
    }
    break;
    case VK_RIGHT:
    {
      ZSharp::InputManager& inputManager = ZSharp::InputManager::Get();
      inputManager.UpdateMiscKey(ZSharp::MiscKey::RIGHT_ARROW, ZSharp::InputManager::KeyState::Up);
    }
    break;
    case VK_DOWN:
    {
      ZSharp::InputManager& inputManager = ZSharp::InputManager::Get();
      inputManager.UpdateMiscKey(ZSharp::MiscKey::DOWN_ARROW, ZSharp::InputManager::KeyState::Up);
    }
    break;
    case VK_RETURN:
    {
      ZSharp::InputManager& inputManager = ZSharp::InputManager::Get();
      inputManager.UpdateMiscKey(ZSharp::MiscKey::ENTER, ZSharp::InputManager::KeyState::Up);
    }
    break;
    case VK_BACK:
    {
      ZSharp::InputManager& inputManager = ZSharp::InputManager::Get();
      inputManager.UpdateMiscKey(ZSharp::MiscKey::BACKSPACE, ZSharp::InputManager::KeyState::Up);
    }
    break;
    default:
    {
      ZSharp::InputManager& inputManager = ZSharp::InputManager::Get();
      inputManager.Update(key, ZSharp::InputManager::KeyState::Up);
    }
      break;
  }
}

void Win32PlatformApplication::OnWindowResize(const RECT* rect) {
  (void)rect;

  // The resize rect passed in is off by a little bit.
  // Calling GetClientRect gets us the true dimensions we need.
  RECT activeWindowSize{};
  if (GetClientRect(mWindowHandle, &activeWindowSize)) {
    UpdateWindowSize(activeWindowSize);
  }
}

void Win32PlatformApplication::OnPreWindowSizeChanged(LPMINMAXINFO info) {
  const ZSharp::ZConfig& config = ZSharp::ZConfig::Get();

  const ZSharp::GameSetting<size_t> width = config.GetViewportWidth();
  const ZSharp::GameSetting<size_t> height = config.GetViewportHeight();

  info->ptMinTrackSize.x = ZSharp::Clamp(info->ptMinTrackSize.x, (LONG)width.Min(), (LONG)width.Max());
  info->ptMaxTrackSize.x = ZSharp::Clamp(info->ptMaxTrackSize.x, (LONG)width.Min(), (LONG)width.Max());
  info->ptMinTrackSize.y = ZSharp::Clamp(info->ptMinTrackSize.y, (LONG)height.Min(), (LONG)height.Max());
  info->ptMaxTrackSize.y = ZSharp::Clamp(info->ptMaxTrackSize.y, (LONG)height.Min(), (LONG)height.Max());
}

void Win32PlatformApplication::OnWindowVisibility(WPARAM param) {
  // Stop rendering if the window becomes minimized since we can't see anything.
  if (param == SIZE_MINIMIZED) {
    mHidden = true;
  }
  else if (param == SIZE_RESTORED) {
    mHidden = false;

    RECT activeWindowSize{};
    if (GetClientRect(mWindowHandle, &activeWindowSize)) {
      UpdateWindowSize(activeWindowSize);
    }
  }
  else if (param == SIZE_MAXIMIZED) {
    RECT activeWindowSize{};
    if (GetClientRect(mWindowHandle, &activeWindowSize)) {
      UpdateWindowSize(activeWindowSize);
    }
  }

  return;
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

  if (mWindowContext != nullptr) {
    ReleaseDC(mWindowHandle, mWindowContext);
  }

  mWindowHandle = nullptr;

  PostQuitMessage(0);
}

void Win32PlatformApplication::UpdateFrame(const ZSharp::uint8* data) {
  ZSharp::NamedScopedTimer(BlitFrame);

  SetDIBitsToDevice(mWindowContext, 
    0, 
    0, 
    mBitmapInfo->bmiHeader.biWidth, 
    -mBitmapInfo->bmiHeader.biHeight, 
    0, 
    0,
    0, 
    -mBitmapInfo->bmiHeader.biHeight,
    data, 
    mBitmapInfo, 
    DIB_RGB_COLORS);

  ValidateRect(mWindowHandle, NULL);
}

void Win32PlatformApplication::SplatTexture(const ZSharp::uint8* data, size_t width, size_t height, size_t bitsPerPixel) {
  if (data == nullptr) {
    return;
  }
  
  BITMAPINFO info{};
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

  SetDIBitsToDevice(mWindowContext,
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

  ValidateRect(mWindowHandle, NULL);
}

void Win32PlatformApplication::UpdateAudio() {
  mGameInstance->TickAudio();
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
    (1000 / (*LockedFPS)) - 2,
    &Win32PlatformApplication::OnTimerThunk,
    this,
    true);
}

void Win32PlatformApplication::UpdateWindowSize(const RECT rect) {
  ZSharp::ZConfig& config = ZSharp::ZConfig::Get();

  bool dirtySize = false;

  ZSharp::int32 width = rect.right;
  ZSharp::int32 height = rect.bottom;

  if (width != config.GetViewportWidth().Value()) {
    config.SetViewportWidth(width);
    dirtySize = true;
  }

  if (height != config.GetViewportHeight().Value()) {
    config.SetViewportHeight(height);
    dirtySize = true;
  }

  mBitmapInfo->bmiHeader.biWidth = width;
  mBitmapInfo->bmiHeader.biHeight = -height;

  if (dirtySize) {
    mGameInstance->WaitForBackgroundJobs();
    ZSharp::OnWindowSizeChangedDelegate().Broadcast(width, height);
  }
}

bool Win32PlatformApplication::IsSpecialKey(ZSharp::int32 key) {
  return key == VK_SPACE || 
    key == VK_ESCAPE ||
    key == VK_UP ||
    key == VK_LEFT ||
    key == VK_RIGHT ||
    key == VK_DOWN ||
    key == VK_RETURN ||
    key == VK_BACK;
}

bool Win32PlatformApplication::IsShiftPressed() {
  return GetAsyncKeyState(VK_LSHIFT) || GetAsyncKeyState(VK_RSHIFT);
}

#endif
