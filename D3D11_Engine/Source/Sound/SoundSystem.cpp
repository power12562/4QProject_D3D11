#include <filesystem>
#include "Sound/SoundSystem.h"

SoundSystem::SoundSystem()
{
	FMOD_RESULT fmod_result;
	fmod_result = FMOD::System_Create(&fmod_system);
	if (fmod_result != FMOD_OK) __debugbreak;
	fmod_result = fmod_system->getVersion(&fmod_version);
	if (fmod_result != FMOD_OK) __debugbreak;
	fmod_result = fmod_system->init(fmod_channel_count, FMOD_INIT_NORMAL, fmod_extra_driver_data);
	if (fmod_result != FMOD_OK) __debugbreak;

	fmod_system->getMasterChannelGroup(&master_group);
}

void SoundSystem::UpdateFMODSystem()
{
	fmod_system->update();
}

void SoundSystem::SetMasterVolume(float volume)
{
	master_group->setVolume(volume);
}

FMOD::Sound* SoundSystem::CreateSound(std::wstring_view path)
{
	std::filesystem::path p(path);
	if (std::filesystem::exists(p) == false)
	{
		__debugbreak;
		return nullptr;
	}

	std::string _path;
	_path.assign(path.begin(), path.end());

	FMOD::Sound* sound{ nullptr };
	FMOD_RESULT fr = fmod_system->createSound(_path.c_str(), FMOD_DEFAULT, 0, &sound);

	if (fr != FMOD_RESULT::FMOD_OK)
	{
		__debugbreak;
		return nullptr;
	}

	return sound;
}
