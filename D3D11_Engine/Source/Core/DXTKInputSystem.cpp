#include "DXTKInputSystem.h"
#include <Utility\D3D11Utility.h>
#include <Core/GameInputSystem.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;

DXTKInputSystem& DXTKinputSystem = DXTKInputSystem::GetInstance();

void DXTKInputSystem::Update()
{
	mouseState = mouse->GetState(); 
	mouseStateTracker.Update(mouseState);
}

void DXTKInputSystem::Initialize(HWND hWnd)
{
	hWnd = hWnd;
	mouse = std::make_unique<Mouse>();
	mouse->SetWindow(hWnd);
	mouse->SetMode(Mouse::MODE_ABSOLUTE);
}

bool DXTKInputSystem::IsKeyDown(MouseKeys key) const
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

bool  DXTKInputSystem::IsKey(MouseKeys key) const
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

bool  DXTKInputSystem::IsKeyUp(MouseKeys key) const
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

void DXTKInputSystem::SetMouseMode(DirectX::Mouse::Mode mode)
{
	mouse->SetMode(mode);
}

const DirectX::Mouse::State& DXTKInputSystem::GetMouseState()
{
	return mouseState;
}
