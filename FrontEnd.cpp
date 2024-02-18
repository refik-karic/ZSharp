#include "FrontEnd.h"

#include "PlatformMemory.h"
#include "ZConfig.h"
#include "ZAssert.h"
#include "Delegate.h"
#include "InputManager.h"
#include "UIButton.h"
#include "UILinearPanel.h"
#include "UILabel.h"
#include "UIGrid.h"
#include "PlatformApplication.h"
#include "ZString.h"
#include "PlatformMisc.h"

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

  UIGridColumn lowerColumn0(.8f, "LowerColumn0");
  UIGridColumn lowerColumn1(.2f, "LowerColumn1");
  UIGridRow lowerRow0(1.f, "LowerRow0");

  UIGrid* lowerGrid = new UIGrid(width, height, "LowerGrid");
  lowerGrid->AddRow(lowerRow0);
  lowerGrid->AddColumn(lowerColumn0);
  lowerGrid->AddColumn(lowerColumn1);
  lowerGrid->GridRow() = 1;

  const String buildType(String::FromFormat("Build: {0}", PlatformGetBuildType()));

  UILabel* labelText0 = new UILabel(150, 10, "DemoLabel0");
  labelText0->SetText(buildType);
  labelText0->SetHighlightColor(ZColors::YELLOW);
  labelText0->HorizontalAlignment() = UIHorizontalAlignment::Left;
  labelText0->VerticalAlignment() = UIVerticalAlignment::Bottom;

  const String userText(String::FromFormat("Player: {0}", PlatformGetUsername()));

  UILabel* labelText1 = new UILabel(150, 10, "DemoLabel1");
  labelText1->SetText(userText);
  labelText1->SetHighlightColor(ZColors::YELLOW);
  labelText1->HorizontalAlignment() = UIHorizontalAlignment::Right;
  labelText1->VerticalAlignment() = UIVerticalAlignment::Bottom;

  UILabel* buttonLabel = new UILabel(100, 10, "ButtonLabel");
  buttonLabel->SetText("Start Game");
  buttonLabel->SetColor(ZColors::ORANGE);
  buttonLabel->VerticalAlignment() = UIVerticalAlignment::Center;
  buttonLabel->HorizontalAlignment() = UIHorizontalAlignment::Center;

  UIButton* testButton = new UIButton(200, 100, "DemoButton");
  testButton->SetLabel(buttonLabel);
  testButton->SetColor(ZColors::BLUE);
  testButton->HorizontalAlignment() = UIHorizontalAlignment::Center;
  testButton->OnClickDelegate = Delegate<void>::FromMember<FrontEnd, &FrontEnd::OnStartButtonClicked>(this);

  UILinearPanel* linearPanel0 = new UILinearPanel(width, height, "DemoPanel0", UILinearFlow::Vertical);
  linearPanel0->AddItem(testButton);
  linearPanel0->HorizontalAlignment() = UIHorizontalAlignment::Center;
  linearPanel0->VerticalAlignment() = UIVerticalAlignment::Center;

  UILinearPanel* linearPanel1 = new UILinearPanel(width, height, "DemoPanel1", UILinearFlow::Vertical);
  linearPanel1->AddItem(labelText0);
  linearPanel1->HorizontalAlignment() = UIHorizontalAlignment::Left;
  linearPanel1->VerticalAlignment() = UIVerticalAlignment::Bottom;
  linearPanel1->GridColumn() = 0;

  UILinearPanel* linearPanel2 = new UILinearPanel(width, height, "DemoPanel2", UILinearFlow::Vertical);
  linearPanel2->AddItem(labelText1);
  linearPanel2->HorizontalAlignment() = UIHorizontalAlignment::Right;
  linearPanel2->VerticalAlignment() = UIVerticalAlignment::Bottom;
  linearPanel2->GridColumn() = 1;

  lowerGrid->AddItem(linearPanel1);
  lowerGrid->AddItem(linearPanel2);

  UIGridColumn column0(1.f, "DemoColumn0");
  UIGridRow row0(.9f, "DemoRow0");
  UIGridRow row1(.1f, "DemoRow1");

  UIGrid* grid = new UIGrid(width, height, "DemoGrid");
  grid->AddItem(linearPanel0);
  grid->AddItem(lowerGrid);
  grid->HorizontalAlignment() = UIHorizontalAlignment::Center;
  grid->VerticalAlignment() = UIVerticalAlignment::Center;
  grid->AddColumn(column0);
  grid->AddRow(row0);
  grid->AddRow(row1);

  mFrame = new UIFrame(width, height, grid);
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
