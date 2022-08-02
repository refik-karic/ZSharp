#include "ZConfig.h"

#include "IniFile.h"
#include "PlatformFile.h"

namespace ZSharp {

ZConfig& ZConfig::GetInstance() {
  static ZConfig singleton;
  return singleton;
}

size_t ZConfig::GetViewportWidth() const {
  return mViewportWidth;
}

size_t ZConfig::GetViewportHeight() const {
  return mViewportHeight;
}

size_t ZConfig::GetViewportStride() const {
  return mViewportStride;
}

size_t ZConfig::GetBytesPerPixel() const {
  return mBytesPerPixel;
}

FileString ZConfig::GetAssetPath() const {
  return mAssetPath;
}

Array<String> ZConfig::GetAssets() const {
  return mAssets;
}

void ZConfig::SetViewportWidth(size_t width) {
  mViewportWidth = width;
  mViewportStride = mBytesPerPixel * width;
}

void ZConfig::SetViewportHeight(size_t height) {
  mViewportHeight = height;
}

void ZConfig::SetBytesPerPixel(size_t bytesPerPixel) {
  mBytesPerPixel = bytesPerPixel;
  mViewportStride = mViewportWidth * bytesPerPixel;
}

bool ZConfig::SizeChanged(size_t width, size_t height) {
  return ((width != mViewportWidth) || (height != mViewportHeight));
}

ZConfig::ZConfig()
  : mAssetPath("") {
  FileString iniFilePath(PlatformGetUserDesktopPath());
  iniFilePath.AddDirectory("models");
  iniFilePath.SetFilename("ZSharpSettings.txt");

  IniFile userConfig(iniFilePath);
  
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
    userConfig.GetAllValuesForSection("SerializedAssets", mAssets);
  }
}

void ZConfig::SetAssetPath(const String& path) {
  mAssetPath = path;
}

}
