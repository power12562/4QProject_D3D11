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
}

void SoundSystem::UpdateFMODSystem()
{
	fmod_system->update();
}
