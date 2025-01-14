#include "PhysicsTestController.h"
#include "framework.h"

void PhysicsTestController::Update()
{
	auto& input = GameInputSystem::GetInstance();
	if (input.IsKeyDown(KeyboardKeys::P))
	{
		AddComponent<Rigidbody>();
		AddComponent<BoxCollider>();
	}
}