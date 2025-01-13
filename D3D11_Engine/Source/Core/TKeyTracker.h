#pragma once
#include <Windows.h>

enum class KeyState { Idle, Pressed, Held, Released };
template <size_t KEY_COUNT>
class TKeyTracker
{
public:
    TKeyTracker() { memset(keyStates, 0, sizeof(KeyState) * KEY_COUNT); }
	~TKeyTracker() = default;

	void Update(const bool(&arr)[KEY_COUNT]);
    inline KeyState GetKeyState(size_t keyIndex) const { return keyStates[keyIndex]; }
private:
	KeyState keyStates[KEY_COUNT]; // 현재 키 상태
};

template<size_t KEY_COUNT>
inline void TKeyTracker<KEY_COUNT>::Update(const bool(&arr)[KEY_COUNT])
{
	for (unsigned int i = 0; i < KEY_COUNT; ++i)
	{
		switch (keyStates[i])
		{
		case KeyState::Pressed:
			keyStates[i] = arr[i] ? KeyState::Held : KeyState::Released;
			break;
		case KeyState::Held:
			keyStates[i] = arr[i] ? KeyState::Held : KeyState::Released;
			break;
		case KeyState::Released:
			keyStates[i] = arr[i] ? KeyState::Pressed : KeyState::Idle;
			break;
		case KeyState::Idle:
			keyStates[i] = arr[i] ? KeyState::Pressed : KeyState::Idle;
			break;
		}
	}
}
