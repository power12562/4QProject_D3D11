#include "Component/SoundManager/SoundManager.h"
#include "Sound/SoundSystem.h"

void SoundManager::SetSound(FMOD::Sound* sound, bool loop)
{
	if (channel != nullptr)
	{
		Stop();
	}

	currentSound = sound;
	if (loop == true)
	{
		currentSound->setMode(FMOD_LOOP_NORMAL);
	}
	else if (loop == false)
	{
		currentSound->setMode(FMOD_LOOP_OFF);
	}
}

void SoundManager::Play()
{
	if (currentSound == nullptr) return;

	SoundSystem::GetInstance().GetFMODSystem()->playSound(currentSound, 0, false, &channel);
	channel->setVolume(volume);
}

void SoundManager::Play(FMOD::Sound* sound, bool loop)
{
	SetSound(sound, loop);
	Play();
}

void SoundManager::Pause()
{
	if (currentSound == nullptr || channel == nullptr) return;

	channel->setPaused(true);
}

void SoundManager::Resume()
{
	if (currentSound == nullptr || channel == nullptr) return;

	channel->setPaused(false);
}

void SoundManager::Stop()
{
	if (currentSound == nullptr || channel == nullptr) return;

	channel->stop();
	channel = nullptr;
}

void SoundManager::SetVolume(float _volume)
{
	_volume = _volume > 1.0f ? 1.0f : _volume;
	_volume = _volume < 0.0f ? 0.0f : _volume;

	this->volume = _volume;

	if (currentSound == nullptr || channel == nullptr) return;
	channel->setVolume(_volume);
}

void SoundManager::VolumeUp(float amount)
{
	volume = volume + amount > 1.0f ? 1.0f : volume + amount;
	
	if (currentSound == nullptr || channel == nullptr) return;

	channel->setVolume(volume);
}

void SoundManager::VolumeDown(float amount)
{
	volume = volume - amount < 0.0f ? 0.0f : volume - amount;

	if (currentSound == nullptr || channel == nullptr) return;

	channel->setVolume(volume);
}

void SoundManager::MuteOn()
{
	if (currentSound == nullptr || channel == nullptr) return;

	channel->setMute(true);
	mute = true;
}

void SoundManager::MuteOff()
{
	if (currentSound == nullptr || channel == nullptr) return;

	channel->setMute(false);
	mute = false;
}

//void SoundManager::SetPitch(float pitch)
//{
//	if (currentSound == nullptr || channel == nullptr) return;
//
//	channel->setPitch(pitch);
//}