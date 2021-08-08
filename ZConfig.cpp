#include "ZConfig.h"

#include <filesystem>

namespace ZSharp {

ZConfig& ZConfig::GetInstance() {
  static ZConfig singleton;
  return singleton;
}

std::size_t ZConfig::GetViewportWidth() const {
  return mViewportWidth;
}

std::size_t ZConfig::GetViewportHeight() const {
  return mViewportHeight;
}

std::size_t ZConfig::GetViewportStride() const {
  return mViewportStride;
}

std::size_t ZConfig::GetBytesPerPixel() const {
  return mBytesPerPixel;
}

void ZConfig::SetViewportWidth(std::size_t width) {
  mViewportWidth = width;
  mViewportStride = mBytesPerPixel * width;
}

void ZConfig::SetViewportHeight(std::size_t height) {
  mViewportHeight = height;
}

void ZConfig::SetBytesPerPixel(std::size_t bytesPerPixel) {
  mBytesPerPixel = bytesPerPixel;
  mViewportStride = mViewportWidth * bytesPerPixel;
}

ZConfig::ZConfig() {
  //std::filesystem::path workingDirectory = std::filesystem::current_path().parent_path();
  

  // Low res
  SetViewportWidth(640);
  SetViewportHeight(480);
  // High res
  //SetViewportWidth(1920);
  //SetViewportHeight(1080);
  SetBytesPerPixel(4);
}

}
