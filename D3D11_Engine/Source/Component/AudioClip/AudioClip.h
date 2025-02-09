#pragma once
#include "Sound/FMODFramework.h"
#include "Component/Base/Component.h"

class AudioClip : public Component
{
	static constexpr float VOLUME_MAX = 100.0f;

	FMOD::Channel* channel{ nullptr };
	FMOD::Sound* currentSound{ nullptr };
	float volume{ 1.0f };

	bool pause{ false };
	bool mute{ false };

public:
	void SetSound(FMOD::Sound*, bool loop = false);

	void Play();
	void Play(FMOD::Sound*, bool loop = false);
	
	void Pause();
	void Resume();
	void Stop();

	void SetVolume(float volume);
	float GetVolume() { return volume; }
	void VolumeUp(float amount);
	void VolumeDown(float amount);
	void MuteOn();
	void MuteOff();

	void SetMasterVolume(float master_volume);
	bool IsPlaying();

	//void SetPitch(float pitch);




public:
	virtual void Start() override {}
protected:
	virtual void FixedUpdate() override {}
	virtual void Update() override {}
	virtual void LateUpdate() override {}
};

using Sound = FMOD::Sound;