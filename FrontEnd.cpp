#include "FrontEnd.h"

#include "PlatformMemory.h"
#include "ZConfig.h"
#include "ZAssert.h"
#include "Delegate.h"
#include "InputManager.h"
#include "UIButton.h"
#include "UILinearPanel.h"
#include "UILabel.h"
#include "PlatformApplication.h"
#include "ZString.h"

namespace ZSharp {

FrontEnd::FrontEnd() {
  OnWindowSizeChangedDelegate().Add(Delegate<size_t, size_t>::FromMember<FrontEnd, &FrontEnd::OnResize>(this));

  InputManager& inputManager = InputManager::Get();
  inputManager.OnMouseMoveDelegate.Add(Delegate<int32, int32, bool>::FromMember<FrontEnd, &FrontEnd::OnMouseMove>(this));
}

FrontEnd::~FrontEnd() {
  if (mFrame != nullptr) {
    delete mFrame;
  }

  OnWindowSizeChangedDelegate().Remove(Delegate<size_t, size_t>::FromMember<FrontEnd, &FrontEnd::OnResize>(this));

  InputManager& inputManager = InputManager::Get();
  inputManager.OnMouseMoveDelegate.Remove(Delegate<int32, int32, bool>::FromMember<FrontEnd, &FrontEnd::OnMouseMove>(this));
}

bool FrontEnd::IsLoaded() {
  return mLoaded;
}

void FrontEnd::Load() {
  mVisible = true;
  mLoaded = true;

  ZConfig& config = ZConfig::Get();

  size_t width = config.GetViewportWidth().Value();
  size_t height = config.GetViewportHeight().Value();

  UILabel* labelText = new UILabel(150, 10, "DemoLabel");
  labelText->SetText("Demo V1");
  labelText->SetHighlightColor(ZColors::YELLOW);
  labelText->HorizontalAlignment() = UIHorizontalAlignment::Center;
  labelText->VerticalAlignment() = UIVerticalAlignment::Center;

  UILabel* buttonLabel = new UILabel(100, 10, "ButtonLabel");
  buttonLabel->SetText("Start Game");
  buttonLabel->SetColor(ZColors::ORANGE);
  buttonLabel->VerticalAlignment() = UIVerticalAlignment::Center;
  buttonLabel->HorizontalAlignment() = UIHorizontalAlignment::Center;

  UIButton* testButton = new UIButton(100, 50, "DemoButton");
  testButton->SetLabel(buttonLabel);
  testButton->SetColor(ZColors::BLUE);
  testButton->HorizontalAlignment() = UIHorizontalAlignment::Center;
  testButton->OnClickDelegate = Delegate<void>::FromMember<FrontEnd, &FrontEnd::OnStartButtonClicked>(this);

  UILinearPanel* linearPanel = new UILinearPanel(width, height, "DemoPanel", UILinearFlow::Vertical);
  linearPanel->AddItem(labelText);
  linearPanel->AddItem(testButton);
  linearPanel->HorizontalAlignment() = UIHorizontalAlignment::Center;
  linearPanel->VerticalAlignment() = UIVerticalAlignment::Center;

  mFrame = new UIFrame(width, height, linearPanel);
}

void FrontEnd::Unload() {
  mVisible = false;
  mLoaded = false;

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

void FrontEnd::OnMouseMove(int32 x, int32 y, bool mouseDown) {
  if (!mVisible) {
    return;
  }

  ZAssert(mFrame != nullptr);
  mFrame->OnMouseMove(x, y, mouseDown);
}

void FrontEnd::OnStartButtonClicked() {
  Unload();
}

}
