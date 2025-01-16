#pragma once
#include "Sound/FMODFramework.h"
#include "Core/TSingleton.h"

class SoundSystem : public TSingleton<SoundSystem>
{
	FMOD::System* fmod_system{ nullptr };
	FMOD::ChannelGroup* master_group{ nullptr };
	unsigned int fmod_channel_count{ 64 };
	unsigned int fmod_version;
	void* fmod_extra_driver_data{ nullptr };

public:
	SoundSystem();
	void UpdateFMODSystem();

	// ±âº»°ª 1.0(100%),
	void SetMasterVolume(float volume);

	FMOD::Sound* CreateSound(std::wstring_view path);

	FMOD::System* GetFMODSystem() { return fmod_system; }
};

