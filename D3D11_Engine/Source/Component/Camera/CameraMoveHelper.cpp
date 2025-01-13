#include "CameraMoveHelper.h"
#include <Core/TimeSystem.h>
#include <Component/Camera/Camera.h>
#include <Math/Mathf.h>
#include <Core/GameInputSystem.h>

CameraMoveHelper::CameraMoveHelper()
{
	rotSpeed = 0.1f;
	moveSpeed = 100.0f;
}

void CameraMoveHelper::Start()
{
	SetDefaultTransform(transform);
}

void CameraMoveHelper::FixedUpdate()
{

}

void CameraMoveHelper::Update()
{
	if (Camera* mainCame = Camera::GetMainCamera())
	{
		if (&mainCame->gameObject == &gameObject)
		{
			inputVector.Normalize();
			if (inputVector.Length() > 0.0f)
			{
				transform.position += inputVector * moveSpeed * TimeSystem::Time.DeltaTime;
				inputVector = Vector3::Zero;
			}
			if (yawRotation)
			{
				transform.rotation = Quaternion::CreateFromAxisAngle(Vector3::UnitX, yawRotation) * transform.rotation;
				yawRotation = 0;
			}
			if (pitchRotation)
			{
				transform.rotation *= Quaternion::CreateFromAxisAngle(Vector3::UnitY, pitchRotation);
				pitchRotation = 0;
			}
			inputVector = Vector3::Zero;
		}
	}
}

void CameraMoveHelper::LateUpdate()
{

}

void CameraMoveHelper::OnInputProcess(InputManager::Input& Input)
{
	if (Input.IsKey(MouseKeys::rightButton))
	{
		Vector3 forward = transform.Forward;
		Vector3 right = transform.Right;

		if (gameInputSystem.IsKey(KeyboardKeys::R))
		{
			Reset();
		}

		if (gameInputSystem.IsKey(KeyboardKeys::W))
		{
			inputVector += forward;
		}
		else if (gameInputSystem.IsKey(KeyboardKeys::S))
		{
			inputVector += -forward;
		}

		if (gameInputSystem.IsKey(KeyboardKeys::A))
		{
			inputVector += -right;
		}
		else if (gameInputSystem.IsKey(KeyboardKeys::D))
		{
			inputVector += right;
		}

		if (gameInputSystem.IsKey(KeyboardKeys::Q))
		{
			inputVector += -transform.Up;
		}
		else if (gameInputSystem.IsKey(KeyboardKeys::E))
		{
			inputVector += transform.Up;
		}

		if (Input.IsKeyDown(KeyboardKeys::F1))
		{
			moveSpeed = 10;
		}
		else if (Input.IsKeyDown(KeyboardKeys::F2))
		{
			moveSpeed = 100;
		}
		else if (Input.IsKeyDown(KeyboardKeys::F3))
		{
			moveSpeed = 1000;
		}

		const Mouse::State& MouseState = Input.GetMouseState();
		if (MouseState.positionMode == Mouse::Mode::MODE_RELATIVE)
		{
			Vector2 delta = Vector2(float(MouseState.x), float(MouseState.y)) * rotSpeed * Mathf::Deg2Rad;
			AddPitch(delta.x);
			AddYaw(delta.y);
		}
		else
		{
			Input.SetMouseMode(Mouse::Mode::MODE_RELATIVE);
		}
	}
	else
	{
		Input.SetMouseMode(Mouse::Mode::MODE_ABSOLUTE);
	}
}

void CameraMoveHelper::SetDefaultTransform(Transform& defaultTransform)
{
	startTransform = defaultTransform;
}

void CameraMoveHelper::Reset()
{
	transform = startTransform;
}

void CameraMoveHelper::AddYaw(float value)
{	
	yawRotation = value;
}

void CameraMoveHelper::AddPitch(float value)
{
	pitchRotation = value;
}
