#include "Manager/SoundManager.h"
#include <Sound/SoundSystem.h>

FMOD::Sound* SoundManager::AddSound(std::wstring_view key, std::wstring_view path)
{
	if (sounds.find(key.data()) == sounds.end())
	{
		FMOD::Sound* _sound = SoundSystem::GetInstance().CreateSound(path);
		sounds[key.data()] = _sound;
		return _sound;
	}
	else
	{
		printf("Resource already exists : FMOD::Sound / %s\n", key.data());
		return sounds[key.data()];
	}
}

FMOD::Sound* SoundManager::GetSound(std::wstring_view key)
{
	if (sounds.find(key.data()) == sounds.end())
	{
		printf("Resource not found : FMOD::Sound / %s\n", key.data());
		return nullptr;
	}
	else
	{
		return sounds[key.data()];
	}
}

void SoundManager::RemoveSound(std::wstring_view key)
{
	if (sounds.find(key.data()) == sounds.end())
	{
		return;
	}
	sounds[key.data()]->release();
	sounds.erase(key.data());
}

SoundManager::~SoundManager()
{
	for (auto& p : sounds)
	{
		p.second->release();
	}
}