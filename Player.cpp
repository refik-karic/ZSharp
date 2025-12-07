#include "Player.h"

#include "CommonMath.h"
#include "ConsoleVariable.h"
#include "Delegate.h"
#include "InputManager.h"
#include "ZConfig.h"

#include <cmath>
#include <cstring>

namespace ZSharp {

ConsoleVariable<float> CameraSpeed("CameraSpeed", 1.f);
ConsoleVariable<float> CameraRotation("CameraRotation", 5.f);

Player::Player(DevConsole* devConsole) : mCamera(new Camera()), mDevConsole(devConsole) {
  // TODO: Need to find a good way to decide on the bounding box for the player.
  const float min[3] = { -5.f, -5.f, -5.f };
  const float max[3] = { 5.f, 5.f, 5.f };
  mBoundingBox = AABB(min, max);

  memset(&mState, 0, sizeof(mState));
}

Player::~Player() {
  if (mCamera) {
    delete mCamera;
  }

  InputManager* inputManager = GlobalInputManager;
  inputManager->OnKeyDownDelegate.Remove(Delegate<uint8>::FromMember<Player, &Player::OnKeyDown>(this));
  inputManager->OnAsyncKeyDownDelegate.Remove(Delegate<uint8>::FromMember<Player, &Player::OnAsyncKeyDown>(this));
  inputManager->OnMouseDragDelegate.Remove(Delegate<int32, int32, int32, int32>::FromMember<Player, &Player::OnMouseMove>(this));
}

void Player::Load() {
  mPosition = { 0.f, 15.f, 50.f };

  InputManager* inputManager = GlobalInputManager;
  inputManager->OnKeyDownDelegate.Add(Delegate<uint8>::FromMember<Player, &Player::OnKeyDown>(this));
  inputManager->OnAsyncKeyDownDelegate.Add(Delegate<uint8>::FromMember<Player, &Player::OnAsyncKeyDown>(this));
  inputManager->OnMouseDragDelegate.Add(Delegate<int32, int32, int32, int32>::FromMember<Player, &Player::OnMouseMove>(this));
}

void Player::Tick() {
  // TODO: Just hacking some stuff together quickly.
  // What actually needs to happen is that player velocity needs to go up in the Y axis and then gradually slowed back down via gravity.
  // We also need some fast reliable way to tell if we're at rest on a surface.
  if (mState.jumping) {
    if (!mState.airborne) {
      mPosition[1] = 15.f;
      mState.airborne = true;
    }
    else {
      if (FloatLessThanEqual(mPosition[1], 10.f)) {
        mState.airborne = false;
        mState.jumping = false;
      }
    }
  }

  mCamera->Position() = mPosition;
  mCamera->RotateCamera(mRotation);
  mCamera->Tick();
}

void Player::MoveCamera(Direction direction) {
  Vec3 cameraLook(mCamera->GetLook());

  cameraLook *= (*CameraSpeed);

  // TODO: Figure out a good way to make physics velocity play nice with camera position/movement.
  switch (direction) {
  case Direction::FORWARD:
    mPosition += cameraLook;
    break;
  case Direction::BACK:
    mPosition -= cameraLook;
    break;
  case Direction::LEFT:
  {
    Vec3 sideVec(mCamera->GetUp().Cross(cameraLook));
    mPosition += sideVec;
  }
  break;
  case Direction::RIGHT:
  {
    Vec3 sideVec(mCamera->GetUp().Cross(cameraLook));
    mPosition -= sideVec;
  }
  break;
  }
}

void Player::RotateCamera(const Quaternion& quat) {
  mRotation = quat;
}

Vec3 Player::ProjectClick(float x, float y) {
  const ZConfig* config = GlobalConfig;
  float width = (float)config->GetViewportWidth().Value();
  float height = (float)config->GetViewportHeight().Value();

  float radius = width * (*CameraRotation);

  float newX = ((x - (width / 2.f)) / radius);
  float newY = ((y - (height / 2.f)) / radius);

  // Note that we flip the X axis so that left/right rotation looks natural.

  float r = (newX * newX) + (newY * newY);
  if (r > 1.f) {
    float s = 1.f / sqrtf(r);
    return { -newX * s, newY * s, 0.f };
  }
  else {
    return { -newX, newY, sqrtf(1.f - r) };
  }
}

void Player::ResetCamera() {
  mCamera->ResetOrientation();
}

Vec3& Player::Position() {
  return mPosition;
}

Camera* Player::ViewCamera() {
  return mCamera;
}

void Player::OnKeyDown(uint8 key) {
  if (mDevConsole->IsOpen()) {
    return;
  }

  switch (key) {
  case ' ':
  {
    mState.jumping = true;
  }
    break;
  }
}

void Player::OnAsyncKeyDown(uint8 key) {
  if (mDevConsole->IsOpen()) {
    return;
  }

  InputManager* input = GlobalInputManager;

  switch (key) {
  case 'w':
  {
    if (input->GetKeyState('a') == InputManager::KeyState::Down) {
      MoveCamera(Player::Direction::LEFT);
    }
    else if (input->GetKeyState('d') == InputManager::KeyState::Down) {
      MoveCamera(Player::Direction::RIGHT);
    }

    MoveCamera(Player::Direction::FORWARD);
  }
  break;
  case 's':
  {
    if (input->GetKeyState('a') == InputManager::KeyState::Down) {
      MoveCamera(Player::Direction::LEFT);
    }
    else if (input->GetKeyState('d') == InputManager::KeyState::Down) {
      MoveCamera(Player::Direction::RIGHT);
    }

    MoveCamera(Player::Direction::BACK);
  }
  break;
  case 'a':
  {
    if (input->GetKeyState('w') == InputManager::KeyState::Down) {
      MoveCamera(Player::Direction::FORWARD);
    }
    else if (input->GetKeyState('s') == InputManager::KeyState::Down) {
      MoveCamera(Player::Direction::BACK);
    }

    MoveCamera(Player::Direction::LEFT);
  }
  break;
  case 'd':
  {
    if (input->GetKeyState('w') == InputManager::KeyState::Down) {
      MoveCamera(Player::Direction::FORWARD);
    }
    else if (input->GetKeyState('s') == InputManager::KeyState::Down) {
      MoveCamera(Player::Direction::BACK);
    }

    MoveCamera(Player::Direction::RIGHT);
  }
  break;
  case 'q':
    RotateCamera(Quaternion(DegreesToRadians(1.f), { 0.f, 1.f, 0.f }));
    break;
  case 'e':
    RotateCamera(Quaternion(DegreesToRadians(-1.f), { 0.f, 1.f, 0.f }));
    break;
  default:
    break;
  }
}

void Player::OnMouseMove(int32 oldX, int32 oldY, int32 x, int32 y) {
  const Vec3 V1(ProjectClick((float)oldX, (float)oldY));
  const Vec3 V2(ProjectClick((float)x, (float)y));

  const Vec3 normal(V1.Cross(V2));
  float theta = V1 * V2;
  const Quaternion quat(theta, normal);
  RotateCamera(quat);
}

}
