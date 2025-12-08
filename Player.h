#pragma once

#include "ZBaseTypes.h"
#include "Camera.h"
#include "PhysicsObject.h"
#include "Vec3.h"

namespace ZSharp {

class Player final : public PhysicsObject {
	public:

	Player();
	~Player();

	enum class Direction {
		LEFT,
		RIGHT,
		FORWARD,
		BACK
	};

	void Load();

	void Tick();

	void MoveCamera(Direction direction);

	void RotateCamera(const Quaternion& quat);

	Vec3 ProjectClick(float x, float y);

	void ResetCamera();

	Vec3& Position();

	Camera* ViewCamera();

	private:
	Camera* mCamera = nullptr;

	struct PlayerState {
		bool jumping : 1;
		bool airborne : 1;
	} mState;

	void OnKeyDown(uint8 key);

	void OnAsyncKeyDown(uint8 key);

	void OnMouseMove(int32 oldX, int32 oldY, int32 x, int32 y);
};

}
