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

#include <synchapi.h>
#include <timeapi.h>
#include <processenv.h>
#include <shellapi.h>

#define DEBUG_TEXTURE_PNG 0
#define DEBUG_TEXTURE_JPG 0

static ZSharp::WideString WindowClassName(L"SoftwareRendererWindowClass");
static ZSharp::WideString TimerName(L"MainLoop");
UINT MinTimerPeriod = 0;
DWORD WindowStyle = WS_BORDER | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_THICKFRAME;

ZSharp::ConsoleVariable<bool> UncappedFPS("UncappedFPS", false);
ZSharp::ConsoleVariable<ZSharp::int32> LockedFPS("LockedFPS", 60);

Win32PlatformApplication* GlobalApplication = nullptr;

namespace ZSharp {

BroadcastDelegate<size_t, size_t>& OnWindowSizeChangedDelegate() {
  static BroadcastDelegate<size_t, size_t> instance;
  return instance;
}

PlatformApplication* GetApplication() {
  return GlobalApplication;
}

}

LRESULT Win32PlatformApplication::MessageLoop(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  Win32PlatformApplication* app = GlobalApplication;

  switch (uMsg) {
  case WM_CREATE:
    app->OnCreate(hwnd);
    return 0;
  case WM_PAINT:
    app->OnPaint();
    return 0;
  case WM_ERASEBKGND:
    return true;
  case WM_LBUTTONDOWN:
    app->OnLButtonDown(LOWORD(lParam), HIWORD(lParam));
    return 0;
  case WM_LBUTTONUP:
    app->OnLButtonUp();
    return 0;
  case WM_MOUSEMOVE:
    app->OnMouseMove(LOWORD(lParam), HIWORD(lParam));
    return 0;
  case WM_KEYDOWN:
    app->TranslateKey(wParam, LOBYTE(HIWORD(lParam)), true);
    return 0;
  case WM_KEYUP:
    app->TranslateKey(wParam, LOBYTE(HIWORD(lParam)), false);
    return 0;
  case WM_GETMINMAXINFO:
    app->OnPreWindowSizeChanged((LPMINMAXINFO)lParam);
    break;
  case WM_SIZE:
    app->OnWindowVisibility(wParam);
    break;
  case WM_SIZING:
    app->OnWindowResize();
    break;
  case WM_CLOSE:
    app->OnClose();
    break;
  case WM_DESTROY:
    app->OnDestroy();
    break;
  case WM_QUIT:
    return wParam;
  default:
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
  }

  return 0;
}

int Win32PlatformApplication::Run(HINSTANCE instance) {
  if (mWindowHandle != nullptr) {
    return -1;
  }

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
    while (PeekMessageW(&msg, mWindowHandle, 0, 0, PM_REMOVE)) {
      DispatchMessageW(&msg);
    }
      
    UpdateAudio();

    SleepEx(MinTimerPeriod, true);
  }

  UnregisterClassW(WindowClassName.Str(), mInstance);
  return 0;
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
  : mBitmapInfo{new BITMAPINFO()}, mKeyboard(new BYTE[256]), mPointCursor(nullptr), mHandCursor(nullptr) {
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

  memset(mKeyboard, 0, 256);

  ZSharp::InitializeGlobals();
  mGameInstance = new ZSharp::GameInstance();
}

Win32PlatformApplication::~Win32PlatformApplication() {
  if (mKeyboard) {
    delete[] mKeyboard;
  }

  if (mBitmapInfo) {
    delete mBitmapInfo;
  }

  if (mGameInstance) {
    delete mGameInstance;
  }

  ZSharp::FreeGlobals();
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

  const ZSharp::ZConfig* config = ZSharp::GlobalConfig;

  RECT clientRect{ 0L, 0L, static_cast<long>(config->GetViewportWidth().Value()), static_cast<long>(config->GetViewportHeight().Value()) };
  AdjustWindowRectEx(&clientRect, WindowStyle, false, WS_EX_OVERLAPPEDWINDOW);

  return CreateWindowExW(
    WS_EX_OVERLAPPEDWINDOW,
    WindowClassName.Str(),
    config->GetWindowTitle().ToWide().Str(),
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
  TIMECAPS timecaps;
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
  RECT activeWindowSize;
  if (GetClientRect(initialHandle, &activeWindowSize)) {
    UpdateWindowSize(&activeWindowSize);
  }

  mWindowContext = GetDC(initialHandle);
  if (mWindowContext == nullptr) {
    DestroyWindow(initialHandle);
    return;
  }

  StartTimer((ZSharp::int64)1, (1000 / (*LockedFPS)));
}

void Win32PlatformApplication::OnTimer(LPVOID optionalArg, DWORD timerLowVal, DWORD timerHighValue) {
  (void)optionalArg;
  (void)timerLowVal;
  (void)timerHighValue;

  Win32PlatformApplication* app = GlobalApplication;
  if (!app->mPaused && !app->mHidden) {
    size_t frameDeltaTime = ZSharp::PlatformHighResClock();

    app->PauseTimer();

    if (app->mCurrentCursor != ZSharp::AppCursor::Arrow) {
      app->ApplyCursor(ZSharp::AppCursor::Arrow);
    }

    app->mGameInstance->Tick();

    InvalidateRect(app->mWindowHandle, NULL, false);

    // Sleep if we have some time left in the frame, otherwise start again immediately.
    frameDeltaTime = ZSharp::PlatformHighResClockDeltaMs(frameDeltaTime);
    const size_t lockedMs = (1000 / (*LockedFPS));
    if (frameDeltaTime >= lockedMs || (*UncappedFPS)) {
      frameDeltaTime = 1;
    }
    else {
      frameDeltaTime = (lockedMs - frameDeltaTime) * 10000;
    }

    app->StartTimer((ZSharp::int64)frameDeltaTime, lockedMs);
  }
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
  ZSharp::InputManager* inputManager = ZSharp::GlobalInputManager;
  inputManager->UpdateMousePosition(x, y);
  inputManager->UpdateMouseState(true);
}

void Win32PlatformApplication::OnLButtonUp() {
  ZSharp::InputManager* inputManager = ZSharp::GlobalInputManager;
  inputManager->UpdateMouseState(false);
}

void Win32PlatformApplication::OnMouseMove(ZSharp::int32 x, ZSharp::int32 y) {
  ZSharp::InputManager* inputManager = ZSharp::GlobalInputManager;
  inputManager->UpdateMousePosition(x, y);
}

void Win32PlatformApplication::OnKeyDown(ZSharp::uint8 key) {
  ZSharp::InputManager* inputManager = ZSharp::GlobalInputManager;

  switch (key) {
  case VK_SPACE:
  {
    if (!mGameInstance->IsDevConsoleOpen()) {
       mPaused = !mPaused;
    }
    else {
      inputManager->Update(key, ZSharp::InputManager::KeyState::Down);
    }
  }
    break;
  case VK_ESCAPE:
    DestroyWindow(mWindowHandle);
    break;
  case VK_UP:
  {
    inputManager->UpdateMiscKey(ZSharp::MiscKey::UP_ARROW, ZSharp::InputManager::KeyState::Down);
  }
    break;
  case VK_LEFT:
  {
    inputManager->UpdateMiscKey(ZSharp::MiscKey::LEFT_ARROW, ZSharp::InputManager::KeyState::Down);
  }
  break;
  case VK_RIGHT:
  {
    inputManager->UpdateMiscKey(ZSharp::MiscKey::RIGHT_ARROW, ZSharp::InputManager::KeyState::Down);
  }
  break;
  case VK_DOWN:
  {
    inputManager->UpdateMiscKey(ZSharp::MiscKey::DOWN_ARROW, ZSharp::InputManager::KeyState::Down);
  }
  break;
  case VK_RETURN:
  {
    inputManager->UpdateMiscKey(ZSharp::MiscKey::ENTER, ZSharp::InputManager::KeyState::Down);
  }
  break;
  case VK_BACK:
  {
    inputManager->UpdateMiscKey(ZSharp::MiscKey::BACKSPACE, ZSharp::InputManager::KeyState::Down);
  }
  break;
  default:
  {
    inputManager->Update(key, ZSharp::InputManager::KeyState::Down);
  }
  break;
  }
}

void Win32PlatformApplication::OnKeyUp(ZSharp::uint8 key) {
  ZSharp::InputManager* inputManager = ZSharp::GlobalInputManager;

  switch (key) {
    case VK_UP:
    {
      inputManager->UpdateMiscKey(ZSharp::MiscKey::UP_ARROW, ZSharp::InputManager::KeyState::Up);
    }
    break;
    case VK_LEFT:
    {
      inputManager->UpdateMiscKey(ZSharp::MiscKey::LEFT_ARROW, ZSharp::InputManager::KeyState::Up);
    }
    break;
    case VK_RIGHT:
    {
      inputManager->UpdateMiscKey(ZSharp::MiscKey::RIGHT_ARROW, ZSharp::InputManager::KeyState::Up);
    }
    break;
    case VK_DOWN:
    {
      inputManager->UpdateMiscKey(ZSharp::MiscKey::DOWN_ARROW, ZSharp::InputManager::KeyState::Up);
    }
    break;
    case VK_RETURN:
    {
      inputManager->UpdateMiscKey(ZSharp::MiscKey::ENTER, ZSharp::InputManager::KeyState::Up);
    }
    break;
    case VK_BACK:
    {
      inputManager->UpdateMiscKey(ZSharp::MiscKey::BACKSPACE, ZSharp::InputManager::KeyState::Up);
    }
    break;
    default:
    {
      inputManager->Update(key, ZSharp::InputManager::KeyState::Up);
    }
      break;
  }
}

void Win32PlatformApplication::OnWindowResize() {
  RECT activeWindowSize;
  if (GetClientRect(mWindowHandle, &activeWindowSize)) {
    UpdateWindowSize(&activeWindowSize);
  }
}

void Win32PlatformApplication::OnPreWindowSizeChanged(LPMINMAXINFO info) {
  const ZSharp::ZConfig* config = ZSharp::GlobalConfig;

  const ZSharp::GameSetting<size_t> width = config->GetViewportWidth();
  const ZSharp::GameSetting<size_t> height = config->GetViewportHeight();

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

    RECT activeWindowSize;
    if (GetClientRect(mWindowHandle, &activeWindowSize)) {
      UpdateWindowSize(&activeWindowSize);
    }
  }
  else if (param == SIZE_MAXIMIZED) {
    RECT activeWindowSize;
    if (GetClientRect(mWindowHandle, &activeWindowSize)) {
      UpdateWindowSize(&activeWindowSize);
    }
  }
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

void Win32PlatformApplication::StartTimer(ZSharp::int64 relativeNanoseconds, size_t lockedMs) {
  LARGE_INTEGER dueTime;
  dueTime.QuadPart = -1 * relativeNanoseconds;

  SetWaitableTimer(mHighPrecisionTimer,
    &dueTime,
    (LONG)(lockedMs - 2),
    &Win32PlatformApplication::OnTimer,
    nullptr,
    true);
}

void Win32PlatformApplication::UpdateWindowSize(const RECT* rect) {
  ZSharp::ZConfig* config = ZSharp::GlobalConfig;

  bool dirtySize = false;

  ZSharp::int32 width = rect->right;
  ZSharp::int32 height = rect->bottom;

  if (width != config->GetViewportWidth().Value()) {
    config->SetViewportWidth(width);
    dirtySize = true;
  }

  if (height != config->GetViewportHeight().Value()) {
    config->SetViewportHeight(height);
    dirtySize = true;
  }

  mBitmapInfo->bmiHeader.biWidth = width;
  mBitmapInfo->bmiHeader.biHeight = -height;

  if (dirtySize) {
    mGameInstance->WaitForBackgroundJobs();
    ZSharp::OnWindowSizeChangedDelegate().Broadcast(width, height);
  }
}

void Win32PlatformApplication::TranslateKey(WPARAM key, WORD scanCode, bool isDown) {
  // NOTE: We must update the keyboard state prior to translation.
  //  This ensures that the state of keys such as CAPS or SHIFT are taken into account.
  //  Each time a virtual key changes we translate and update its state on the keyboard.
  mKeyboard[(BYTE)key] = (BYTE)GetKeyState((int)key);
  
  if (IsSpecialKey((ZSharp::int32)key)) {
    ZSharp::uint8 inputKey = static_cast<ZSharp::uint8>(key);
    if (isDown) {
      OnKeyDown(inputKey);
    }
    else {
      OnKeyUp(inputKey);
    }
  }
  else {
    // TODO: This assumes a simple US ASCII keyboard.
    //  For internationalization we would need to take into account the locale and use ToUnicode() rather than ToAscii().
    UINT uKeyCode = (UINT)key;
    WORD translatedKey;
    if(ToAscii(uKeyCode, scanCode, mKeyboard, &translatedKey, 0)) {
      ZSharp::uint8 inputKey = static_cast<ZSharp::uint8>(translatedKey);
      if (isDown) {
        OnKeyDown(inputKey);
      }
      else {
        OnKeyUp(inputKey);
      }
    }
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

#endif
