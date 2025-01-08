#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <Utility/AssimpUtility.h>
#include <directxtk/Keyboard.h>
#include <functional>
#include <queue>

class GameObject;
class Scene
{
	friend class SceneManager;
	using InstanceID = unsigned int;
public:
	Scene();
	virtual void Start() {}

	virtual ~Scene();
protected:
	virtual void ImGUIRender() {}

private:
	static void ImGUIBegineDraw();
	void ImGuizmoDraw();
	std::queue<std::function<void()>> ImGUIPopupQue;
	static void ImGUIEndDraw();

public:
	bool UseImGUI = false;
	
public:
	const wchar_t* GetSceneName() { return sceneName.c_str(); }

private:
	std::wstring sceneName;

private:
	std::vector<std::shared_ptr<GameObject>> objectList;
	std::vector<std::weak_ptr<GameObject>>   dontdestroyonloadList;
private:
	//Update
	void FixedUpdate();
	void Update();
	void LateUpdate();

	//Render
	void Render();

public:
	inline static struct ImGuizmoSetting
	{
		bool UseImGuizmo = false;
		GameObject* SelectObject = nullptr;
		int operation = 7;		//ImGuizmo::OPERATION::TRANSLATE
		int mode = 1;			//ImGuizmo::WORLD
		struct 
		{
			DirectX::Keyboard::Keys TRANSLATE = DirectX::Keyboard::Keys::W;
			DirectX::Keyboard::Keys ROTATE = DirectX::Keyboard::Keys::E;
			DirectX::Keyboard::Keys SCALE = DirectX::Keyboard::Keys::R;
			DirectX::Keyboard::Keys UNIVERSAL = DirectX::Keyboard::Keys::T;
			DirectX::Keyboard::Keys MODE = DirectX::Keyboard::Keys::X;
		}KeySetting;
	}
	GuizmoSetting;
};
