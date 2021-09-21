#include "ZConfig.h"

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

bool ZConfig::SizeChanged(std::size_t width, std::size_t height) {
  return ((width != mViewportWidth) || (height != mViewportHeight));
}

ZConfig::ZConfig() {
  SetViewportWidth(1920);
  SetViewportHeight(1080);
  SetBytesPerPixel(4);
}

}
