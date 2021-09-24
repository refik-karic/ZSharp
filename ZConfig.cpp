#include "ZConfig.h"

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

ZConfig::ZConfig() {
  SetViewportWidth(1920);
  SetViewportHeight(1080);
  SetBytesPerPixel(4);
}

}
