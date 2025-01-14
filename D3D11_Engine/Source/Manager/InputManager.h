#pragma once
#include <Core/DXTKInputSystem.h>
#include <Core/GameInputSystem.h>

class InputManager;
extern InputManager& inputManager;
class InputManager : public TSingleton<InputManager>
{
	friend class TSingleton;
	friend class InputProcesser;
public:
	void Initialize();
	void Uninitialize();
	void UpdateProcesser();

public:
	struct Input
	{
		bool IsKeyDown(KeyboardKeys key) const;
		bool IsKey(KeyboardKeys key) const;
		bool IsKeyUp(KeyboardKeys key) const;

		bool IsKeyDown(MouseKeys key) const;
		bool IsKey(MouseKeys key) const;
		bool IsKeyUp(MouseKeys key) const;
		void SetMouseMode(DirectX::Mouse::Mode mode);
		const DirectX::Mouse::State& GetMouseState();
	}
	input;
private:
	std::vector<InputProcesser*> inputProcessersList;
};

class InputProcesser
{
	friend class InputManager;
protected:
	InputProcesser();
	~InputProcesser();

protected:
	virtual void OnInputProcess(InputManager::Input& input) = 0;
};