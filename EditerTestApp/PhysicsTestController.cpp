#include "PhysicsTestController.h"
#include "framework.h"

void PhysicsTestController::Start()
{
	auto nyan = GetResourceManager<Sound>().AddResource(L"sample_sound", L"./Resource/Sound/nyan_cat.mp3");
	auto& sm = AddComponent<SoundManager>();
	sm.SetSound(nyan, false);
	sm.Play();
	sm.SetVolume(0.33f);
}

void PhysicsTestController::Update()
{
	auto& input = GameInputSystem::GetInstance();
	if (input.IsKeyDown(KeyboardKeys::P))
	{
		AddComponent<Rigidbody>();
		AddComponent<BoxCollider>();
	}
}