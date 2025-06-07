#include "ZConfig.h"

#include "IniFile.h"
#include "PlatformFile.h"

namespace ZSharp {

ZConfig& ZConfig::Get() {
  static ZConfig singleton;
  return singleton;
}

GameSetting<size_t> ZConfig::GetViewportWidth() const {
  return mViewportWidth;
}

GameSetting<size_t> ZConfig::GetViewportHeight() const {
  return mViewportHeight;
}

size_t ZConfig::GetViewportStride() const {
  return mViewportStride;
}

GameSetting<size_t> ZConfig::GetBytesPerPixel() const {
  return mBytesPerPixel;
}

FileString ZConfig::GetAssetPath() const {
  return mAssetPath;
}

Array<String> ZConfig::GetAssets() const {
  return mAssets;
}

const String& ZConfig::GetWindowTitle() const {
  return mWindowTitle.Value();
}

void ZConfig::SetViewportWidth(size_t width) {
  mViewportWidth = width;
  mViewportStride = mBytesPerPixel.Value() * width;
}

void ZConfig::SetViewportHeight(size_t height) {
  mViewportHeight = height;
}

void ZConfig::SetBytesPerPixel(size_t bytesPerPixel) {
  mBytesPerPixel = bytesPerPixel;
  mViewportStride = mViewportWidth.Value() * bytesPerPixel;
}

bool ZConfig::SizeChanged(size_t width, size_t height) {
  return ((width != mViewportWidth.Value()) || (height != mViewportHeight.Value()));
}

ZConfig::ZConfig()
  :  
  mViewportWidth(640, 3840, 1920),
  mViewportHeight(480, 2160, 1080),
  mBytesPerPixel(4, 4, 4),
  mViewportStride(0),
  mAssetPath(""),
  mWindowTitle("Software_Renderer_V3") {
  FileString iniFilePath(PlatformGetUserDataDirectory());
  iniFilePath.SetFilename("ZSharpSettings.txt");

  IniFile userConfig(iniFilePath);
  if (!userConfig.Loaded()) {
    // Force viewport stride to update.
    SetViewportWidth(mViewportWidth.Value());
    return;
  }
  
  {
    String viewportWidth(userConfig.FindValue("GlobalSettings", "ViewportWidth"));
    if (!viewportWidth.IsEmpty()) {
      SetViewportWidth(viewportWidth.ToUint32());
    }
  }
  
  {
    String viewportHeight(userConfig.FindValue("GlobalSettings", "ViewportHeight"));
    if (!viewportHeight.IsEmpty()) {
      SetViewportHeight(viewportHeight.ToUint32());
    }
  }

  {
    String bytesPerPixel(userConfig.FindValue("GlobalSettings", "BytesPerPixel"));
    if (!bytesPerPixel.IsEmpty()) {
      SetBytesPerPixel(bytesPerPixel.ToUint32());
    }
  }

  {
    String assetPath(userConfig.FindValue("GlobalSettings", "AssetPath"));
    if (!assetPath.IsEmpty()) {
      SetAssetPath(assetPath);
    }
  }

  {
    String windowTitle(userConfig.FindValue("GlobalSettings", "WindowTitle"));
    if (!windowTitle.IsEmpty()) {
      SetWindowTitle(windowTitle);
    }
  }

  {
    userConfig.GetAllValuesForSection("SerializedAssets", mAssets);
  }
}

void ZConfig::SetAssetPath(const String& path) {
  mAssetPath = path;
}

void ZConfig::SetWindowTitle(const String& title) {
  mWindowTitle = title;
}

}
