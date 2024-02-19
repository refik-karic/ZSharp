#include "UIBase.h"

namespace ZSharp {

UIBase::UIBase() 
  : mWidth(0), mHeight(0), mName(""), mX(0), mY(0), mGridRow(0), mGridColumn(0), mBorderThickness(0), mBorderColor(0), mMouseOver(false) {
}

UIBase::UIBase(size_t width, size_t height, const String& name)
  : mWidth(width), mHeight(height), mName(name), mX(0), mY(0), mGridRow(0), mGridColumn(0), mBorderThickness(0), mBorderColor(0), mMouseOver(false)  {
}

void UIBase::Layout(size_t x, size_t y) {
  (void)x;
  (void)y;
}

void UIBase::Draw(uint8* screen, size_t width, size_t height) {
  (void)screen;
  (void)width;
  (void)height;
}

void UIBase::Resize(size_t width, size_t height) {
  mWidth = width;
  mHeight = height;
}

bool UIBase::operator==(const UIBase& rhs) const {
  return mName == rhs.mName;
}

size_t UIBase::GetWidth() {
  return mWidth;
}

void UIBase::SetWidth(size_t width) {
  mWidth = width;
}

const size_t UIBase::GetWidth() const {
  return mWidth;
}

size_t UIBase::GetHeight() {
  return mHeight;
}

void UIBase::SetHeight(size_t height) {
  mHeight = height;
}

const size_t UIBase::GetHeight() const {
  return mHeight;
}

size_t UIBase::GetGridRow() {
  return mGridRow;
}

void UIBase::SetGridRow(size_t row) {
  mGridRow = row;
}

const size_t UIBase::GetGridRow() const {
  return mGridRow;
}

size_t UIBase::GetGridColumn() {
  return mGridColumn;
}

void UIBase::SetGridColumn(size_t column) {
  mGridColumn = column;
}

const size_t UIBase::GetGridColumn() const {
  return mGridColumn;
}

void UIBase::SetBorderThickness(size_t size) {
  mBorderThickness = size;
}

void UIBase::SetBorderColor(const ZColor& color) {
  mBorderColor = color;
}

void UIBase::SetHighlightBorderColor(const ZColor& color) {
  mBorderHighlightColor = color;
}

UIHorizontalAlignment& UIBase::HorizontalAlignment() {
  return mHorizontalAlignment;
}

UIVerticalAlignment& UIBase::VerticalAlignment() {
  return mVerticalAlignment;
}

void UIBase::SetMouseOver(bool state) {
  mMouseOver = state;
}

void UIBase::DrawBorder(uint8* buffer, size_t bufferWidth) {
  size_t borderX = mX;
  size_t borderY = mY;

  uint32* pixelOffset = (uint32*)(buffer + (borderY * bufferWidth * 4) + borderX * 4);

  uint32 colorValue = mMouseOver ? mBorderHighlightColor.Color() : mBorderColor.Color();

  size_t borderWidth = mWidth + (mBorderThickness * 2);

  // Top
  for (size_t y = 0; y < mBorderThickness; ++y) {
    for (size_t x = 0; x < borderWidth; ++x) {
      pixelOffset[(y * bufferWidth) + x] = colorValue;
    }
  }

  size_t xBorderOffset = mWidth + mBorderThickness;
  size_t xBorderOffsetEnd = mWidth + (mBorderThickness * 2);
  size_t yBorderOffset = mHeight - mBorderThickness;
  for (size_t y = mBorderThickness; y < yBorderOffset; ++y) {
    // Left
    for (size_t x = 0; x < mBorderThickness; ++x) {
      pixelOffset[(y * bufferWidth) + x] = colorValue;
    }

    // Right
    for (size_t x = xBorderOffset; x < xBorderOffsetEnd; ++x) {
      pixelOffset[(y * bufferWidth) + x] = colorValue;
    }
  }

  // Bottom
  borderY += yBorderOffset;
  pixelOffset = (uint32*)(buffer + (borderY * bufferWidth * 4) + borderX * 4);

  for (size_t y = 0; y < mBorderThickness; ++y) {
    for (size_t x = 0; x < borderWidth; ++x) {
      pixelOffset[(y * bufferWidth) + x] = colorValue;
    }
  }
}

void UIBase::DrawRect(uint8* buffer, size_t bufferWidth, size_t rWidth, size_t rHeight, const ZColor& color) {
  if (mBorderThickness <= rWidth && mBorderThickness <= rHeight) {
    rHeight -= (mBorderThickness * 2);
  }

  uint32* pixelOffset = (uint32*)(buffer + ((mY + mBorderThickness) * bufferWidth * 4) + (mX + mBorderThickness) * 4);

  uint32 colorValue = color.Color();

  for (size_t y = 0; y < rHeight; ++y) {
    for (size_t x = 0; x < rWidth; ++x) {
      pixelOffset[(y * bufferWidth) + x] = colorValue;
    }
  }
}

void UIBase::HitTest(int32 x, int32 y, bool mouseDown) {
  (void)x;
  (void)y;
  (void)mouseDown;
}

}
