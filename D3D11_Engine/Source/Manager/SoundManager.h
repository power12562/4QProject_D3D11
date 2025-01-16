#pragma once
#include <string>
#include <map>
#include <Core/TSingleton.h>
#include <Sound/FMODFramework.h>

class SoundManager : public TSingleton<SoundManager>
{
	friend class TSingleton<SoundManager>;

	std::map<std::wstring, FMOD::Sound*> sounds;

public:
	FMOD::Sound* AddSound(std::wstring_view key, std::wstring_view path);
	FMOD::Sound* GetSound(std::wstring_view key);
	void RemoveSound(std::wstring_view key);
private:
	SoundManager() {}
	~SoundManager();
};