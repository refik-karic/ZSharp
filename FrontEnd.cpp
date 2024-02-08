#include "FrontEnd.h"

#include "PlatformMemory.h"
#include "ZConfig.h"
#include "ZAssert.h"
#include "Delegate.h"
#include "UILinearPanel.h"
#include "UILabel.h"
#include "Win32PlatformApplication.h"

namespace ZSharp {

FrontEnd::FrontEnd() {
  Win32PlatformApplication::OnWindowSizeChangedDelegate.Add(Delegate<size_t, size_t>::FromMember<FrontEnd, &FrontEnd::OnResize>(this));
}

FrontEnd::~FrontEnd() {
  if (mFrame != nullptr) {
    delete mFrame;
  }

  Win32PlatformApplication::OnWindowSizeChangedDelegate.Add(Delegate<size_t, size_t>::FromMember<FrontEnd, &FrontEnd::OnResize>(this));
}

void FrontEnd::Load() {
  mVisible = true;

  ZConfig& config = ZConfig::Get();

  size_t width = config.GetViewportWidth().Value();
  size_t height = config.GetViewportHeight().Value();

  UILabel* labelText = new UILabel(100, 10, "TestLabel");
  labelText->Text() = "This is a test.";
  UILinearPanel* linearPanel = new UILinearPanel(width, height, "TestPanel", UILinearFlow::Vertical);
  linearPanel->AddItem(labelText);
  linearPanel->HorizontalAlignment() = UIHorizontalAlignment::Center;
  linearPanel->VerticalAlignment() = UIVerticalAlignment::Center;

  mFrame = new UIFrame(width, height, linearPanel);
}

void FrontEnd::Unload() {
  mVisible = false;

  delete mFrame;
  mFrame = nullptr;
}

void FrontEnd::Tick() {
}

bool FrontEnd::IsVisible() const {
  return mVisible;
}

void FrontEnd::Draw(uint8* screen, size_t width, size_t height) {
  mFrame->Draw(screen, width, height);
}

void FrontEnd::OnResize(size_t width, size_t height) {
  if (!mVisible) {
    return;
  }

  ZAssert(mFrame != nullptr);
  mFrame->OnResize(width, height);
}

}
