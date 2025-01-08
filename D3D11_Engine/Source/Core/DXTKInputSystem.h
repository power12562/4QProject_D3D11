#pragma once
#include <Windows.h>
#include <Core/TSingleton.h>
#include <directXTK/Mouse.h>
#include <directXTK/Keyboard.h>
#include <vector>
#include <windef.h>

using namespace DirectX;
class DXTKInputSystem;
extern DXTKInputSystem& DXTKinputSystem;

using KeyboardKeys = DirectX::Keyboard::Keys;
enum class MouseKeys
{
	leftButton,
	middleButton,
	rightButton,
	xButton1,
	xButton2
};
class DXTKInputSystem : public TSingleton<DXTKInputSystem>
{
	friend class TSingleton;
	friend class InputProcesser;
public:
	class InputSystem 
	{
		friend class DXTKInputSystem;
	public:
		bool IsKeyDown(KeyboardKeys key) const;
		bool IsKey(KeyboardKeys key) const;
		bool IsKeyUp(KeyboardKeys key) const;

		bool IsKeyDown(MouseKeys key) const;
		bool IsKey(MouseKeys key) const;
		bool IsKeyUp(MouseKeys key) const;

		void SetMouseMode(DirectX::Mouse::Mode mode);
		const DirectX::Mouse::State& GetMouseState();
	private:
		HWND hWnd{};
		std::unique_ptr<DirectX::Keyboard>              keyboard;
		std::unique_ptr<DirectX::Mouse>                 mouse;

		DirectX::Mouse::State                           mouseState{};

		DirectX::Keyboard::State                        keyboardState{};

		DirectX::Keyboard::KeyboardStateTracker         keyboardStateTracker;
		DirectX::Mouse::ButtonStateTracker              mouseStateTracker;
	}
	Input;
private:
	DXTKInputSystem();
	~DXTKInputSystem() = default;
	std::vector<InputProcesser*>	inputProcessersList;

public:
	void Update();
	void Initialize(HWND hWnd);
};

class InputProcesser
{
	friend class DXTKInputSystem;
protected:
	InputProcesser();
	~InputProcesser();

protected:
	virtual void OnInputProcess(DXTKInputSystem::InputSystem& input) = 0;
};