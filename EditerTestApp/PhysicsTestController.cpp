#include "PhysicsTestController.h"
#include "framework.h"

void PhysicsTestController::Update()
{
	auto& input = DXTKInputSystem::GetInstance();
	if (input.Input.IsKeyDown(KeyboardKeys::Space))
	{
		AddComponent<Rigidbody>();
		AddComponent<BoxCollider>();
	}
}