#include "Component/SoundManager/SoundManager.h"
#include "Sound/SoundSystem.h"

void SoundManager::SetSound(std::shared_ptr<FMOD::Sound> sound, bool loop)
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
	if (currentSound != nullptr && pause == true)
	{
		Resume();
		return;
	}

	if (currentSound == nullptr) return;

	SoundSystem::GetInstance().GetFMODSystem()->playSound(currentSound.get(), 0, false, &channel);
	channel->setVolume(volume);
}

void SoundManager::Play(std::shared_ptr<FMOD::Sound> sound, bool loop)
{
	SetSound(sound, loop);
	Play();
}

void SoundManager::Pause()
{
	if (currentSound == nullptr || channel == nullptr) return;
	if (pause == true) return;

	channel->setPaused(true);
	pause = true;
}

void SoundManager::Resume()
{
	if (currentSound == nullptr || channel == nullptr) return;
	if (pause == false) return;

	channel->setPaused(false);
	pause = false;
}

void SoundManager::Stop()
{
	if (currentSound == nullptr || channel == nullptr) return;

	channel->stop();
	channel = nullptr;
	pause = false;
}

void SoundManager::SetVolume(float _volume)
{
	if (volume == _volume) return;

	_volume = _volume > VOLUME_MAX ? VOLUME_MAX : _volume;
	_volume = _volume < 0.0f ? 0.0f : _volume;

	this->volume = _volume;

	if (currentSound == nullptr || channel == nullptr) return;
	channel->setVolume(_volume);
}

void SoundManager::VolumeUp(float amount)
{
	volume = volume + amount > VOLUME_MAX ? VOLUME_MAX : volume + amount;
	
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

void SoundManager::SetMasterVolume(float master_volume)
{
	SoundSystem::GetInstance().SetMasterVolume(master_volume);
}

//void SoundManager::SetPitch(float pitch)
//{
//	if (currentSound == nullptr || channel == nullptr) return;
//
//	channel->setPitch(pitch);
//}