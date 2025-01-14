#include "GameInputSystem.h"
#include <Utility/D3D11Utility.h>
#include <Utility/Console.h>

GameInputSystem& gameInputSystem = gameInputSystem.GetInstance();

using namespace Utility;

void GameInputSystem::Initialize()
{
	HRESULT result = GameInputCreate(&gameInput);
	CheckHRESULT(result);
}

void GameInputSystem::Uninitialize()
{
	SafeRelease(gameInput);
}

void GameInputSystem::Update()
{
	UpdateKeyboard();
}

bool GameInputSystem::IsKeyDown(KeyboardKeys key) const
{
	return keyboardStatetracker.GetKeyState(static_cast<size_t>(key)) == KeyState::Pressed;
}

bool GameInputSystem::IsKey(KeyboardKeys key) const
{
	return keyboardStatetracker.GetKeyState(static_cast<size_t>(key)) == KeyState::Held 
		|| keyboardStatetracker.GetKeyState(static_cast<size_t>(key)) == KeyState::Pressed;
}

bool GameInputSystem::IsKeyUp(KeyboardKeys key) const
{
	return keyboardStatetracker.GetKeyState(static_cast<size_t>(key)) == KeyState::Released;
}

void GameInputSystem::UpdateKeyboard()
{
	ComPtr<IGameInputReading> reading;
	memset(currKeyStates, 0, sizeof(bool) * KEYBOARD_COUNT);
	if (FAILED(gameInput->GetCurrentReading(GameInputKindKeyboard, nullptr, &reading)))
		return;

	if (reading)
	{
		uint32_t keyCount = reading->GetKeyCount();
		if (keyCount > 0)
		{
			keyboardStateArray.resize(keyCount);
			reading->GetKeyState(keyCount, keyboardStateArray.data());
			for (uint32_t i = 0; i < keyCount; i++)
			{
				currKeyStates[keyboardStateArray[i].virtualKey] = true;
			}
		}
	}
	keyboardStatetracker.Update(currKeyStates);
}
