#include "DXTKInputSystem.h"
#include <Utility\D3D11Utility.h>


using namespace DirectX;
using namespace DirectX::SimpleMath;

DXTKInputSystem& DXTKinputSystem = DXTKInputSystem::GetInstance();

InputProcesser::InputProcesser()
{
	//DXTKinputSystem.inputProcessersList.push_back(dynamic_cast<InputProcesser*>(this));
	DXTKinputSystem.inputProcessersList.push_back(this);
}

InputProcesser::~InputProcesser()
{
   auto& inputList = DXTKinputSystem.inputProcessersList;
   if(!inputList.empty())
	inputList.erase(std::find(inputList.begin(), inputList.end(), this));
}

DXTKInputSystem::DXTKInputSystem()
{
	
}

void DXTKInputSystem::Update()
{
	Input.mouseState = Input.mouse->GetState(); 
	Input.mouseStateTracker.Update(Input.mouseState);

	Input.keyboardState = Input.keyboard->GetState();
	Input.keyboardStateTracker.Update(Input.keyboardState);

	if (!inputProcessersList.empty())
	{
		int count = (int)inputProcessersList.size();
		for (int i = 0; i < count; ++i)
		{
			inputProcessersList[i]->OnInputProcess(Input);
		}
	}
}

void DXTKInputSystem::Initialize(HWND hWnd)
{
	Input.hWnd = hWnd;
	Input.keyboard = std::make_unique<Keyboard>();
	Input.mouse = std::make_unique<Mouse>();
	Input.mouse->SetWindow(hWnd);
	Input.mouse->SetMode(Mouse::MODE_ABSOLUTE);
}



bool DXTKInputSystem::InputSystem::IsKeyDown(KeyboardKeys key) const
{
	return keyboardStateTracker.IsKeyPressed(key);
}

bool DXTKInputSystem::InputSystem::IsKey(KeyboardKeys key) const
{
	return keyboardState.IsKeyDown(key);
}

bool DXTKInputSystem::InputSystem::IsKeyUp(KeyboardKeys key) const
{
	return keyboardStateTracker.IsKeyReleased(key);
}

bool DXTKInputSystem::InputSystem::IsKeyDown(MouseKeys key) const
{
	switch (key)
	{
	case MouseKeys::leftButton:
		return mouseStateTracker.leftButton == Mouse::ButtonStateTracker::PRESSED;
	case MouseKeys::middleButton:
		return mouseStateTracker.middleButton == Mouse::ButtonStateTracker::PRESSED;
	case MouseKeys::rightButton:
		return mouseStateTracker.rightButton == Mouse::ButtonStateTracker::PRESSED;
	case MouseKeys::xButton1:
		return mouseStateTracker.xButton1 == Mouse::ButtonStateTracker::PRESSED;
	case MouseKeys::xButton2:
		return mouseStateTracker.xButton2 == Mouse::ButtonStateTracker::PRESSED;
	default:
		return false;
	}
}

bool  DXTKInputSystem::InputSystem::IsKey(MouseKeys key) const
{
	switch (key)
	{
	case MouseKeys::leftButton:
		return mouseStateTracker.leftButton == Mouse::ButtonStateTracker::HELD;
	case MouseKeys::middleButton:
		return mouseStateTracker.middleButton == Mouse::ButtonStateTracker::HELD;
	case MouseKeys::rightButton:
		return mouseStateTracker.rightButton == Mouse::ButtonStateTracker::HELD;
	case MouseKeys::xButton1:
		return mouseStateTracker.xButton1 == Mouse::ButtonStateTracker::HELD;
	case MouseKeys::xButton2:
		return mouseStateTracker.xButton2 == Mouse::ButtonStateTracker::HELD;
	default:
		return false;
	}
}

bool  DXTKInputSystem::InputSystem::IsKeyUp(MouseKeys key) const
{
	switch (key)
	{
	case MouseKeys::leftButton:
		return mouseStateTracker.leftButton == Mouse::ButtonStateTracker::RELEASED;
	case MouseKeys::middleButton:
		return mouseStateTracker.middleButton == Mouse::ButtonStateTracker::RELEASED;
	case MouseKeys::rightButton:
		return mouseStateTracker.rightButton == Mouse::ButtonStateTracker::RELEASED;
	case MouseKeys::xButton1:
		return mouseStateTracker.xButton1 == Mouse::ButtonStateTracker::RELEASED;
	case MouseKeys::xButton2:
		return mouseStateTracker.xButton2 == Mouse::ButtonStateTracker::RELEASED;
	default:
		return false;
	}
}

void DXTKInputSystem::InputSystem::SetMouseMode(DirectX::Mouse::Mode mode)
{
	mouse->SetMode(mode);
}

const DirectX::Mouse::State& DXTKInputSystem::InputSystem::GetMouseState()
{
	return mouseState;
}
