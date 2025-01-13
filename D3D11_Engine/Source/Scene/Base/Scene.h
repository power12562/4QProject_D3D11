#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <map>
#include <Utility/AssimpUtility.h>
#include <functional>
#include <queue>
#include <Manager/InputManager.h>

class GameObject;
class Scene
{
	friend class SceneManager;
	friend class GameObjectFactory;
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

	/*·ÎµåµÈ ¾À °ü¸®¿ë ¸Ê*/
	std::unordered_map<std::wstring, std::vector<std::weak_ptr<GameObject>>> loadScenesMap;
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
			KeyboardKeys TRANSLATE	= KeyboardKeys::W;
			KeyboardKeys ROTATE		= KeyboardKeys::E;
			KeyboardKeys SCALE		= KeyboardKeys::R;
			KeyboardKeys UNIVERSAL	= KeyboardKeys::T;
			KeyboardKeys MODE		= KeyboardKeys::X;
		}KeySetting;
	}
	GuizmoSetting;
};
