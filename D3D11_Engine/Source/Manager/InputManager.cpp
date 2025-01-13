#include "InputManager.h"

InputManager& inputManager = InputManager::GetInstance();

DXTKInputSystem::DXTKInputSystem()
{
	
}

void InputManager::Initialize()
{
	DXTKinputSystem.Initialize(WinGameApp::GetHWND());
	gameInputSystem.Initialize();
}

void InputManager::Uninitialize()
{
	gameInputSystem.Uninitialize();
}

void InputManager::UpdateProcesser()
{
	DXTKinputSystem.Update();
	gameInputSystem.Update();

	if (!inputProcessersList.empty())
	{
		int count = (int)inputProcessersList.size();
		for (int i = 0; i < count; ++i)
		{
			inputProcessersList[i]->OnInputProcess(input);
		}
	}
}

bool InputManager::Input::IsKeyDown(KeyboardKeys key) const
{
	return gameInputSystem.IsKeyDown(key);
}

bool InputManager::Input::IsKey(KeyboardKeys key) const
{
	return gameInputSystem.IsKey(key);
}

bool InputManager::Input::IsKeyUp(KeyboardKeys key) const
{
	return gameInputSystem.IsKeyUp(key);
}

bool InputManager::Input::IsKeyDown(MouseKeys key) const
{
	return DXTKinputSystem.IsKeyDown(key);
}

bool InputManager::Input::IsKey(MouseKeys key) const
{
	return DXTKinputSystem.IsKey(key);
}

bool InputManager::Input::IsKeyUp(MouseKeys key) const
{
	return DXTKinputSystem.IsKeyUp(key);
}

void InputManager::Input::SetMouseMode(DirectX::Mouse::Mode mode)
{
	DXTKinputSystem.SetMouseMode(mode);
}

const DirectX::Mouse::State& InputManager::Input::GetMouseState()
{
	return DXTKinputSystem.GetMouseState();
}

InputProcesser::InputProcesser()
{
	inputManager.inputProcessersList.push_back(this);
}

InputProcesser::~InputProcesser()
{
	auto& inputList = inputManager.inputProcessersList;
	if (!inputList.empty())
		inputList.erase(std::find(inputList.begin(), inputList.end(), this));
}