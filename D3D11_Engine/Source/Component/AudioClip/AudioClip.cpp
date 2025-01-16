#include "Component/AudioClip/AudioClip.h"
#include "Sound/SoundSystem.h"

void AudioClip::SetSound(FMOD::Sound* sound, bool loop)
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

void AudioClip::Play()
{
	if (currentSound != nullptr && pause == true)
	{
		Resume();
		return;
	}

	if (currentSound == nullptr) return;

	SoundSystem::GetInstance().GetFMODSystem()->playSound(currentSound, 0, false, &channel);
	channel->setVolume(volume);
}

void AudioClip::Play(FMOD::Sound* sound, bool loop)
{
	SetSound(sound, loop);
	Play();
}

void AudioClip::Pause()
{
	if (currentSound == nullptr || channel == nullptr) return;
	if (pause == true) return;

	channel->setPaused(true);
	pause = true;
}

void AudioClip::Resume()
{
	if (currentSound == nullptr || channel == nullptr) return;
	if (pause == false) return;

	channel->setPaused(false);
	pause = false;
}

void AudioClip::Stop()
{
	if (currentSound == nullptr || channel == nullptr) return;

	channel->stop();
	channel = nullptr;
	pause = false;
}

void AudioClip::SetVolume(float _volume)
{
	if (volume == _volume) return;

	_volume = _volume > VOLUME_MAX ? VOLUME_MAX : _volume;
	_volume = _volume < 0.0f ? 0.0f : _volume;

	this->volume = _volume;

	if (currentSound == nullptr || channel == nullptr) return;
	channel->setVolume(_volume);
}

void AudioClip::VolumeUp(float amount)
{
	volume = volume + amount > VOLUME_MAX ? VOLUME_MAX : volume + amount;
	
	if (currentSound == nullptr || channel == nullptr) return;

	channel->setVolume(volume);
}

void AudioClip::VolumeDown(float amount)
{
	volume = volume - amount < 0.0f ? 0.0f : volume - amount;

	if (currentSound == nullptr || channel == nullptr) return;

	channel->setVolume(volume);
}

void AudioClip::MuteOn()
{
	if (currentSound == nullptr || channel == nullptr) return;

	channel->setMute(true);
	mute = true;
}

void AudioClip::MuteOff()
{
	if (currentSound == nullptr || channel == nullptr) return;

	channel->setMute(false);
	mute = false;
}

void AudioClip::SetMasterVolume(float master_volume)
{
	SoundSystem::GetInstance().SetMasterVolume(master_volume);
}

bool AudioClip::IsPlaying()
{
	if (currentSound == nullptr || channel == nullptr) return false;

	bool is_playing{ false };
	channel->isPlaying(&is_playing);
	return is_playing;
}
//void AudioClip::SetPitch(float pitch)
//{
//	if (currentSound == nullptr || channel == nullptr) return;
//
//	channel->setPitch(pitch);
//}