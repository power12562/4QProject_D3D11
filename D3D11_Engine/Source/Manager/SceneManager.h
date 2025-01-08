#pragma once
#include <Core/TSingleton.h>
#include <Utility/AssimpUtility.h>
#include <unordered_map>
#include <string>
#include <functional>
#include <queue>
#include <set>
#include <Utility/utfConvert.h>
#include <Scene\Base\Scene.h>

class Component;
extern class SceneManager& sceneManager;
using ObjectList = std::vector<GameObject*>;
class SceneManager : public TSingleton<SceneManager>
{
	friend TSingleton;
	friend class WinGameApp;
	friend class D3D11_GameApp;
	friend class GameObject;
	friend LRESULT CALLBACK ImGUIWndProcDefault(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	SceneManager();
	~SceneManager();

	std::unique_ptr<Scene> currScene;
	std::unique_ptr<Scene> nextScene;

	std::queue<std::shared_ptr<GameObject>> currAddQueue; //게임 오브젝트 추가 대기열
	std::queue<std::shared_ptr<GameObject>>	nextAddQueue; //게임 오브젝트 추가 대기열

	std::set<GameObject*> eraseSet;		  //게임 오브젝트 삭제 대기열
	std::set<Component*> eraseComponentSet;	      //컴포넌트 삭제 대기열

	std::list<std::pair<std::wstring, GameObject*>> resourceObjectList; //리소스로 등록할 오브젝트 대기열

	std::map<std::wstring, std::unordered_set<Scene::InstanceID>> objectFindMap;

	//씬 초기화 중인지 확인용
	bool isStartScene = false;
public:
	template <typename T>
	void LoadScene();

	/*현재 씬에 오브젝트 추가*/
	void AddGameObject(std::shared_ptr<GameObject>& object);

	void DestroyObject(GameObject* obj);
	void DestroyObject(GameObject& obj);

	void DontDestroyOnLoad(GameObject* obj);
	void DontDestroyOnLoad(GameObject& obj);

	GameObject* FindObject(const wchar_t* name);
	GameObject* GetObjectToID(unsigned int instanceID);
	std::vector<GameObject*> FindObjects(const wchar_t* name);

	size_t GetObjectsCount();
	ObjectList GetObjectList();
	
	/*현재 씬 Imgui 사용 여부 반환*/
	bool IsImGuiActive();

	/*ImGui 팝업 함수 등록 *반드시 함수 내에서 PopImGuiPopupFunc()로 팝업 종료해야함.*/
	void PushImGuiPopupFunc(const std::function<void()>& func);
	void PopImGuiPopupFunc();

	/**Loding 화면 함수 등록.*/
	void SetLodingImguiFunc(const std::function<void()>& func);
	void EndLodingImguiFunc();

	bool EndGame = false;
private:
	//Update
	void FixedUpdateScene();
	void UpdateScene();
	void LateUpdateScene();

	//Render
	void RenderScene();
	void AddObjects();
	void EraseObjects();
	void ChangeScene();
	
private:
	void AddObjectCurrScene(std::shared_ptr<GameObject>& obj);
	void AddObjectNextScene(std::shared_ptr<GameObject>& obj);
	void ChangeObjectName(unsigned int instanceID, const std::wstring& _pervName, const std::wstring& _newName);
	void EraseObject(unsigned int id);
	unsigned int EraseObjectFindMap(GameObject* obj);

	std::function<void()> ImGuiLodingFunc;
};

template<typename T>
inline void SceneManager::LoadScene()
{
	static_assert(std::is_base_of_v<Scene, T>, "T is not Scene");
	if (currScene && !currScene->ImGUIPopupQue.empty())
	{
		return;
	}
	nextScene.reset(new T);
	nextScene->sceneName = utfConvert::utf8_to_wstring(typeid(T).name());
	resourceObjectList.clear();
}
