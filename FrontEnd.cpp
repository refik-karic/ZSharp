#include "FrontEnd.h"

#include "PlatformMemory.h"
#include "ZConfig.h"
#include "ZAssert.h"
#include "Delegate.h"
#include "InputManager.h"
#include "UIButton.h"
#include "UILinearPanel.h"
#include "UILabel.h"
#include "Win32PlatformApplication.h"
#include "ZString.h"

namespace ZSharp {

FrontEnd::FrontEnd() {
  Win32PlatformApplication::OnWindowSizeChangedDelegate.Add(Delegate<size_t, size_t>::FromMember<FrontEnd, &FrontEnd::OnResize>(this));

  InputManager& inputManager = InputManager::Get();
  inputManager.OnMouseMoveDelegate.Add(Delegate<int32, int32>::FromMember<FrontEnd, &FrontEnd::OnMouseMove>(this));
}

FrontEnd::~FrontEnd() {
  if (mFrame != nullptr) {
    delete mFrame;
  }

  Win32PlatformApplication::OnWindowSizeChangedDelegate.Remove(Delegate<size_t, size_t>::FromMember<FrontEnd, &FrontEnd::OnResize>(this));

  InputManager& inputManager = InputManager::Get();
  inputManager.OnMouseMoveDelegate.Remove(Delegate<int32, int32>::FromMember<FrontEnd, &FrontEnd::OnMouseMove>(this));
}

void FrontEnd::Load() {
  mVisible = true;

  ZConfig& config = ZConfig::Get();

  size_t width = config.GetViewportWidth().Value();
  size_t height = config.GetViewportHeight().Value();

  UILabel* labelText = new UILabel(150, 10, "TestLabel");
  labelText->SetText("This is a test.");

  UILabel* buttonLabel = new UILabel(100, 10, "ButtonLabelTest");
  buttonLabel->SetText("Click");
  buttonLabel->SetColor(ZColors::ORANGE);

  UIButton* testButton = new UIButton(100, 100, "TestButton");
  testButton->SetLabel(buttonLabel);
  testButton->SetColor(ZColors::BLUE);

  UILinearPanel* linearPanel = new UILinearPanel(width, height, "TestPanel", UILinearFlow::Horizontal);
  linearPanel->AddItem(labelText);
  linearPanel->AddItem(testButton);
  linearPanel->HorizontalAlignment() = UIHorizontalAlignment::Right;
  linearPanel->VerticalAlignment() = UIVerticalAlignment::Bottom;

  mFrame = new UIFrame(width, height, linearPanel);
}

void FrontEnd::Unload() {
  mVisible = false;

  delete mFrame;
  mFrame = nullptr;
}

void FrontEnd::Tick() {
  mFrame->Layout();
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

void FrontEnd::OnMouseMove(int32 x, int32 y) {
  if (!mVisible) {
    return;
  }

  ZAssert(mFrame != nullptr);
  mFrame->OnMouseMove(x, y);
}

}
