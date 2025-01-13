#pragma once
#include <Core/TSingleton.h>
#include <Core/TKeyTracker.h>
#include <D3DCore/D3D11_GameApp.h>
#include <wrl/client.h>
#include <unordered_set>
#include <vector>
#include <GameInput.h>
#pragma comment(lib,"GameInput.lib")

using namespace Microsoft::WRL;
class GameInputSystem;
extern GameInputSystem& gameInputSystem;

using KeyboardKeys = KeyboardVK;

class GameInputSystem : public TSingleton<GameInputSystem>
{
	friend class TSingleton;
	GameInputSystem() = default;
	~GameInputSystem() = default;

public:
	void Initialize();
	void Uninitialize();
	void Update();

	bool IsKeyDown(KeyboardKeys key) const;
	bool IsKey(KeyboardKeys key) const;
	bool IsKeyUp(KeyboardKeys key) const;

private:
	void UpdateKeyboard();

private:
	IGameInput* gameInput = nullptr; //singletone gameinput factory

private:
	static constexpr size_t KEYBOARD_COUNT = 255;
	std::vector<GameInputKeyState> keyboardStateArray;
	bool currKeyStates[KEYBOARD_COUNT]{};
	TKeyTracker<KEYBOARD_COUNT> keyboardStatetracker;

	
};