#include "PhysicsTestController.h"
#include "framework.h"
#include <algorithm>
#include <random>

void PhysicsTestController::Start()
{
	auto nyan = GetResourceManager<Sound>().AddResource(L"sample_sound1", L"./Resource/Sound/nyan_cat.mp3");
	sounds.push_back(nyan);
	auto viva = GetResourceManager<Sound>().AddResource(L"sample_sound2", L"./Resource/Sound/Coldplay - Viva La Vida (Official Video).mp3");
	sounds.push_back(viva);
	auto happy = GetResourceManager<Sound>().AddResource(L"sample_sound3", L"./Resource/Sound/Pharrell Williams - Happy (Video).mp3");
	sounds.push_back(happy);
	auto africa = GetResourceManager<Sound>().AddResource(L"sample_sound4", L"./Resource/Sound/Shakira - Waka Waka (This Time for Africa) (The Official 2010 FIFA World Cup Song).mp3");
	sounds.push_back(africa);

	sm = &(AddComponent<SoundManager>());
	sm->SetVolume(0.33f);

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(sounds.begin(), sounds.end(), g);
}

void PhysicsTestController::Update()
{
	auto& input = GameInputSystem::GetInstance();
	if (input.IsKeyDown(KeyboardKeys::P) && press_p == false)
	{
		press_p = true;
		AddComponent<Rigidbody>();
		AddComponent<BoxCollider>();
	}

	if (sm->IsPlaying() == false)
	{
		music_idx = music_idx + 1 >= sounds.size() ? 0 : music_idx + 1;
		sm->Play(sounds[music_idx], false);
	}
}