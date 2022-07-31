#if defined(_WIN64)

#include "Win32PlatformApplication.h"

#include "InputManager.h"
#include "Renderer.h"
#include "ZConfig.h"

static wchar_t WINDOW_CLASS_NAME[] = L"SoftwareRendererWindowClass";
static wchar_t WINDOW_TITLE[] = L"Software Renderer";
static constexpr UINT FRAMERATE_60_HZ_MS = 1000 / 60;

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
  case WM_TIMER:
    app.OnTimer();
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

    mWindowHandle = SetupWindow();
    if (mWindowHandle == nullptr) {
      DWORD error = GetLastError();
      HRESULT result = HRESULT_FROM_WIN32(error);
      return result;
    }

    ShowWindow(mWindowHandle, SW_SHOW);
    for (MSG msg; GetMessageW(&msg, mWindowHandle, 0, 0) > 0;) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }

    UnregisterClassW(WINDOW_CLASS_NAME, mInstance);
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
    WINDOW_CLASS_NAME,
    nullptr
  };

  if (!RegisterClassExW(&wc)) {
    return nullptr;
  }

  const ZSharp::ZConfig& config = ZSharp::ZConfig::GetInstance();

  DWORD windowStyle = WS_BORDER | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_THICKFRAME;
  RECT clientRect{ 0L, 0L, static_cast<long>(config.GetViewportWidth()), static_cast<long>(config.GetViewportHeight()) };
  AdjustWindowRectEx(&clientRect, windowStyle, false, WS_EX_OVERLAPPEDWINDOW);

  return CreateWindowExW(
    WS_EX_OVERLAPPEDWINDOW,
    WINDOW_CLASS_NAME,
    WINDOW_TITLE,
    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
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
  mWindowsFrameTimer = SetTimer(initialHandle, 1, FRAMERATE_60_HZ_MS, NULL);

  if (mWindowsFrameTimer == 0) {
    DestroyWindow(initialHandle);
  }
}

void Win32PlatformApplication::OnTimer() {
  RECT activeWindowSize;
  GetClientRect(mWindowHandle, &activeWindowSize);
  InvalidateRect(mWindowHandle, &activeWindowSize, false);
}

void Win32PlatformApplication::OnPaint() {
  ZSharp::ZConfig& config = ZSharp::ZConfig::GetInstance();
  static ZSharp::Renderer renderer(config.GetViewportWidth(), config.GetViewportHeight(), config.GetViewportStride());

  RECT activeWindowSize;
  if (GetClientRect(mWindowHandle, &activeWindowSize)) {
    if (activeWindowSize.right != config.GetViewportWidth()) {
      config.SetViewportWidth(activeWindowSize.right);
    }

    if (activeWindowSize.bottom != config.GetViewportHeight()) {
      config.SetViewportHeight(activeWindowSize.bottom);
    }

    mBitmapInfo.bmiHeader.biWidth = activeWindowSize.right;
    mBitmapInfo.bmiHeader.biHeight = activeWindowSize.bottom;
  }

  UpdateFrame(renderer.RenderNextFrame());
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
    if (mWindowsFrameTimer == 0) {
      mWindowsFrameTimer = SetTimer(mWindowHandle, 1, FRAMERATE_60_HZ_MS, NULL);

      if (mWindowsFrameTimer == 0) {
        DestroyWindow(mWindowHandle);
      }
    }
    else {
      KillTimer(mWindowHandle, mWindowsFrameTimer);
      mWindowsFrameTimer = 0;
    }
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

void Win32PlatformApplication::OnClose() {
  DestroyWindow(mWindowHandle);
}

void Win32PlatformApplication::OnDestroy() {
  KillTimer(mWindowHandle, mWindowsFrameTimer);
  PostQuitMessage(0);
}

void Win32PlatformApplication::UpdateFrame(const ZSharp::uint8* data) {
  PAINTSTRUCT ps;
  HDC hdc = BeginPaint(mWindowHandle, &ps);

  SetDIBitsToDevice(hdc, 
    0, 
    0, 
    mBitmapInfo.bmiHeader.biWidth, 
    mBitmapInfo.bmiHeader.biHeight, 
    0, 
    0,
    0, 
    mBitmapInfo.bmiHeader.biHeight,
    data, 
    &mBitmapInfo, 
    DIB_RGB_COLORS);

  EndPaint(mWindowHandle, &ps);
}

#endif
