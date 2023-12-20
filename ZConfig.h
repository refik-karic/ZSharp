#pragma once

#include "Array.h"
#include "ZBaseTypes.h"
#include "CommonMath.h"
#include "FileString.h"
#include "ZString.h"

namespace ZSharp{

template<typename T>
class GameSetting final {
  public:

  GameSetting(const T& defaultValue)
    : mDefault(defaultValue), mValue(defaultValue), mShouldClamp(false) {

  }

  GameSetting(const T& min, const T& max, const T& defaultValue) 
    : mMin(min), mMax(max), mDefault(defaultValue), mValue(defaultValue), mShouldClamp(true) {

  }

  const T& Value() const {
    return mValue;
  }

  void operator=(const T& value) {
    mValue = value;
    if (mShouldClamp) {
      mValue = Clamp(mValue, mMin, mMax);
    }
  }

  const T& Min() const {
    return mMin;
  }

  const T& Max() const {
    return mMax;
  }

  private:
  T mMin;
  T mMax;
  T mDefault;

  T mValue;
  bool mShouldClamp;
};

class ZConfig final {
  public:

  ZConfig(const ZConfig&) = delete;
  void operator=(const ZConfig&) = delete;

  static ZConfig& Get();

  GameSetting<size_t> GetViewportWidth() const;

  GameSetting<size_t> GetViewportHeight() const;

  size_t GetViewportStride() const;

  GameSetting<size_t> GetBytesPerPixel() const;

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

  GameSetting<size_t> mViewportWidth;
  GameSetting<size_t> mViewportHeight;
  GameSetting<size_t> mBytesPerPixel;
  size_t mViewportStride;

  FileString mAssetPath;

  Array<String> mAssets;

  GameSetting<String> mWindowTitle;
};
}
