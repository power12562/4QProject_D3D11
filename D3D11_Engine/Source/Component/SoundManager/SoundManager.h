#pragma once
#include "Sound/FMODFramework.h"
#include "Component/Base/Component.h"

class SoundManager : public Component
{
	FMOD::Channel* channel{ nullptr };
	FMOD::Sound* currentSound{ nullptr };
	float volume{ 1.0f };

	bool mute{ false };

public:
	void SetSound(FMOD::Sound* sound, bool loop = false);

	void Play();
	void Play(FMOD::Sound* sound, bool loop = false);
	
	void Pause();
	void Resume();
	void Stop();

	void SetVolume(float volume);
	float GetVolume() { return volume; }
	void VolumeUp(float amount);
	void VolumeDown(float amount);
	void MuteOn();
	void MuteOff();

	//void SetPitch(float pitch);
};