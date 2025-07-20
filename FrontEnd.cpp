#include "FrontEnd.h"

#include "PlatformMemory.h"
#include "ZConfig.h"
#include "ZAssert.h"
#include "Bundle.h"
#include "Delegate.h"
#include "InputManager.h"
#include "UIButton.h"
#include "UILinearPanel.h"
#include "UILabel.h"
#include "UIGrid.h"
#include "PlatformApplication.h"
#include "TexturePool.h"
#include "ZString.h"
#include "PlatformMisc.h"

namespace ZSharp {

FrontEnd::FrontEnd() {
  OnWindowSizeChangedDelegate().Add(Delegate<size_t, size_t>::FromMember<FrontEnd, &FrontEnd::OnResize>(this));

  InputManager* inputManager = GlobalInputManager;
  inputManager->OnMouseMoveDelegate.Add(Delegate<int32, int32, bool>::FromMember<FrontEnd, &FrontEnd::OnMouseMove>(this));
}

FrontEnd::~FrontEnd() {
  if (mFrame != nullptr) {
    delete mFrame;
  }

  OnWindowSizeChangedDelegate().Remove(Delegate<size_t, size_t>::FromMember<FrontEnd, &FrontEnd::OnResize>(this));

  InputManager* inputManager = GlobalInputManager;
  inputManager->OnMouseMoveDelegate.Remove(Delegate<int32, int32, bool>::FromMember<FrontEnd, &FrontEnd::OnMouseMove>(this));
}

bool FrontEnd::IsLoaded() {
  return mLoaded;
}

void FrontEnd::Load() {
  mVisible = true;
  mLoaded = true;

  ZConfig* config = GlobalConfig;

  size_t width = config->GetViewportWidth().Value();
  size_t height = config->GetViewportHeight().Value();

  UIGridColumn lowerColumn0(.8f, "LowerColumn0");
  UIGridColumn lowerColumn1(.2f, "LowerColumn1");
  UIGridRow lowerRow0(1.f, "LowerRow0");

  UIGrid* lowerGrid = new UIGrid(width, height, "LowerGrid");
  lowerGrid->AddRow(lowerRow0);
  lowerGrid->AddColumn(lowerColumn0);
  lowerGrid->AddColumn(lowerColumn1);
  lowerGrid->SetGridRow(2);

  const String buildType(String::FromFormat("Build: {0}", PlatformGetBuildType()));

  UILabel* labelText0 = new UILabel(150, 10, "DemoLabel0");
  labelText0->SetText(buildType);
  labelText0->SetBackgroundColor(ZColors::WHITE);
  labelText0->SetHighlightColor(ZColors::YELLOW);
  labelText0->HorizontalAlignment() = UIHorizontalAlignment::Left;
  labelText0->VerticalAlignment() = UIVerticalAlignment::Bottom;

  const String userText(String::FromFormat("Player: {0}", PlatformGetUsername()));

  UILabel* labelText1 = new UILabel(150, 10, "DemoLabel1");
  labelText1->SetText(userText);
  labelText1->SetBackgroundColor(ZColors::WHITE);
  labelText1->SetHighlightColor(ZColors::YELLOW);
  labelText1->HorizontalAlignment() = UIHorizontalAlignment::Right;
  labelText1->VerticalAlignment() = UIVerticalAlignment::Bottom;

  UILabel* buttonLabel0 = new UILabel(100, 10, "ButtonLabel0");
  buttonLabel0->SetText("PLAY");
  buttonLabel0->SetBackgroundColor(ZColors::BLACK);
  buttonLabel0->SetHighlightColor(ZColors::BLACK);
  buttonLabel0->VerticalAlignment() = UIVerticalAlignment::Center;
  buttonLabel0->HorizontalAlignment() = UIHorizontalAlignment::Center;

  UILabel* buttonLabel1 = new UILabel(100, 10, "ButtonLabel1");
  buttonLabel1->SetText("QUIT");
  buttonLabel1->SetBackgroundColor(ZColors::BLACK);
  buttonLabel1->SetHighlightColor(ZColors::BLACK);
  buttonLabel1->VerticalAlignment() = UIVerticalAlignment::Center;
  buttonLabel1->HorizontalAlignment() = UIHorizontalAlignment::Center;

  UIButton* testButton0 = new UIButton(200, 100, "DemoButton0");
  testButton0->SetLabel(buttonLabel0);
  testButton0->SetBackgroundColor(ZColors::GRAY);
  testButton0->SetHighlightColor(ZColors::WHITE);
  testButton0->HorizontalAlignment() = UIHorizontalAlignment::Center;
  testButton0->OnClickDelegate = Delegate<void>::FromMember<FrontEnd, &FrontEnd::OnStartButtonClicked>(this);
  testButton0->SetBorderThickness(2);
  testButton0->SetBorderColor(ZColors::WHITE);
  testButton0->SetHighlightBorderColor(ZColors::BLUE);

  UIButton* testButton1 = new UIButton(200, 100, "DemoButton1");
  testButton1->SetLabel(buttonLabel1);
  testButton1->SetBackgroundColor(ZColors::GRAY);
  testButton1->SetHighlightColor(ZColors::WHITE);
  testButton1->HorizontalAlignment() = UIHorizontalAlignment::Center;
  testButton1->OnClickDelegate = Delegate<void>::FromMember<FrontEnd, &FrontEnd::OnQuitButtonClicked>(this);
  testButton1->SetBorderThickness(2);
  testButton1->SetBorderColor(ZColors::WHITE);
  testButton1->SetHighlightBorderColor(ZColors::BLUE);

  UILinearPanel* linearPanel0 = new UILinearPanel(width, height, "DemoPanel0", UILinearFlow::Horizontal);
  linearPanel0->AddItem(testButton0);
  linearPanel0->AddItem(testButton1);
  linearPanel0->SetSpacing(100);
  linearPanel0->HorizontalAlignment() = UIHorizontalAlignment::Center;
  linearPanel0->VerticalAlignment() = UIVerticalAlignment::Center;

  UILabel* topPanelLabel = new UILabel(100, 10, "TopPanelLabel");
  topPanelLabel->SetText("A fully software rendered demo by Refik Karic.");
  topPanelLabel->SetBackgroundColor(ZColors::WHITE);
  topPanelLabel->SetHighlightColor(ZColors::YELLOW);
  topPanelLabel->VerticalAlignment() = UIVerticalAlignment::Center;
  topPanelLabel->HorizontalAlignment() = UIHorizontalAlignment::Center;

  UILinearPanel* centerPanel = new UILinearPanel(width, height, "CenterPanel", UILinearFlow::Vertical);
  centerPanel->AddItem(topPanelLabel);
  centerPanel->HorizontalAlignment() = UIHorizontalAlignment::Center;
  centerPanel->VerticalAlignment() = UIVerticalAlignment::Top;
  centerPanel->SetGridRow(1);

  UILinearPanel* linearPanel1 = new UILinearPanel(width, height, "DemoPanel1", UILinearFlow::Vertical);
  linearPanel1->AddItem(labelText0);
  linearPanel1->HorizontalAlignment() = UIHorizontalAlignment::Left;
  linearPanel1->VerticalAlignment() = UIVerticalAlignment::Bottom;
  linearPanel1->SetGridColumn(0);

  UILinearPanel* linearPanel2 = new UILinearPanel(width, height, "DemoPanel2", UILinearFlow::Vertical);
  linearPanel2->AddItem(labelText1);
  linearPanel2->HorizontalAlignment() = UIHorizontalAlignment::Right;
  linearPanel2->VerticalAlignment() = UIVerticalAlignment::Bottom;
  linearPanel2->SetGridColumn(1);

  lowerGrid->AddItem(linearPanel1);
  lowerGrid->AddItem(linearPanel2);

  UIGridColumn column0(1.f, "DemoColumn0");
  UIGridRow row0(.7f, "DemoRow0");
  UIGridRow row1(.2f, "DemoRow1");
  UIGridRow row2(.1f, "DemoRow2");

  UIImage* backgroundImage = new UIImage(width, height, "DemoBackgroundImage");
  backgroundImage->SetTextureId(LoadBackgroundImage("mainmenu_background"));

  UIGrid* grid = new UIGrid(width, height, "DemoGrid");
  grid->SetBackgroundImage(backgroundImage);
  grid->AddItem(linearPanel0);
  grid->AddItem(centerPanel);
  grid->AddItem(lowerGrid);
  grid->HorizontalAlignment() = UIHorizontalAlignment::Center;
  grid->VerticalAlignment() = UIVerticalAlignment::Center;
  grid->AddColumn(column0);
  grid->AddRow(row0);
  grid->AddRow(row1);
  grid->AddRow(row2);

  mFrame = new UIFrame(width, height, grid);
}

void FrontEnd::Unload() {
  mVisible = false;
  mLoaded = false;
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

  if (!mLoaded) {
    delete mFrame;
    mFrame = nullptr;
  }
}

void FrontEnd::OnStartButtonClicked() {
  Unload();
}

void FrontEnd::OnQuitButtonClicked() {
  GetApplication()->Shutdown();
}

int32 FrontEnd::LoadBackgroundImage(const String& imageName) {
  Bundle* bundle = GlobalBundle;
  Asset* textureAsset = bundle->GetAsset(imageName);

  if (textureAsset == nullptr) {
    ZAssert(false);
    return - 1;
  }

  return GlobalTexturePool->LoadTexture(*textureAsset);
}

}
