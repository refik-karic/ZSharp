#pragma once

#include "Array.h"
#include "ZBaseTypes.h"
#include "FileString.h"
#include "ZString.h"

namespace ZSharp{

class ZConfig final {
  public:

  ZConfig(const ZConfig&) = delete;
  void operator=(const ZConfig&) = delete;

  static ZConfig& GetInstance();

  size_t GetViewportWidth() const;

  size_t GetViewportHeight() const;

  size_t GetViewportStride() const;

  size_t GetBytesPerPixel() const;

  FileString GetAssetPath() const;

  Array<String> GetAssets() const;

  const String& GetWindowTitle() const;

  void SetViewportWidth(size_t width);

  void SetViewportHeight(size_t height);

  void SetBytesPerPixel(size_t bytesPerPixel);

  bool SizeChanged(size_t width, size_t height);

  private:

  ZConfig();

  void SetAssetPath(const String& path);

  void SetWindowTitle(const String& title);

  size_t mViewportWidth = 0;
  size_t mViewportHeight = 0;
  size_t mBytesPerPixel = 0;
  size_t mViewportStride = 0;

  FileString mAssetPath;

  Array<String> mAssets;

  String mWindowTitle;
};
}
