#include "SceneManager.h"
#include <GameObject\Base\CameraObject.h>
#include <Core/DXTKInputSystem.h>
#include <D3DCore/D3D11_GameApp.h>
#include <Component/Render/MeshRender.h>

SceneManager& sceneManager = SceneManager::GetInstance();

void SceneManager::AddGameObject(std::shared_ptr<GameObject>& object)
{
	if (sceneManager.nextScene)
		sceneManager.nextAddQueue.push(object);
	else
		sceneManager.currAddQueue.push(object);

	if (!isStartScene)
		AddObjects();
}

SceneManager::SceneManager()
{
	
}

SceneManager::~SceneManager()
{

}

void SceneManager::DestroyObject(GameObject* obj)
{
	eraseSet.insert(obj);
	if (obj->transform.GetChildCount() > 0)
	{
		for (Transform* childObj : obj->transform.childList)
		{
			DestroyObject(childObj->_gameObject);
		}
	}
}

void SceneManager::DestroyObject(GameObject& obj)
{
	DestroyObject(&obj);
}

void SceneManager::DontDestroyOnLoad(GameObject* obj)
{
	if (nextScene)
	{
		nextScene->dontdestroyonloadList.emplace_back(obj->GetWeakPtr());
	}
	else
	{
		currScene->dontdestroyonloadList.emplace_back(obj->GetWeakPtr());
	}

	if (obj->transform.GetChildCount() > 0)
	{
		for (Transform* childObj : obj->transform.childList)
		{
			DontDestroyOnLoad(childObj->_gameObject);
		}
	}
}

void SceneManager::DontDestroyOnLoad(GameObject& obj)
{
	DontDestroyOnLoad(&obj);
}

size_t SceneManager::GetObjectsCount()
{
	if (currScene)
	{
		size_t count = 0;
		for (auto& item : currScene->objectList)
		{
			if (item.get())
				++count;
		}
		return count;
	}
	else return 0;
}

ObjectList SceneManager::GetObjectList()
{
	if (currScene)
	{
		std::vector<GameObject*> objList;
		objList.reserve((GetObjectsCount()));
		for (int i = 0; i < currScene->objectList.size(); i++)
		{
			if (GameObject* obj = currScene->objectList[i].get())
			{
				objList.push_back(obj);
			}
		}
		return objList;
	}
	else return std::vector<GameObject*>();
}

bool SceneManager::IsImGuiActive()
{
	if (currScene)
		return currScene->UseImGUI;
	else
		return false;
}

void SceneManager::PushImGuiPopupFunc(const std::function<void()>& func)
{
	if (nextScene)
		nextScene->ImGUIPopupQue.push(func);
	else if (currScene)
		currScene->ImGUIPopupQue.push(func);
}

void SceneManager::PopImGuiPopupFunc()
{
	if (currScene)
		currScene->ImGUIPopupQue.pop();
}

void SceneManager::SetLodingImguiFunc(const std::function<void()>& func)
{
	ImGuiLodingFunc = func;
}

void SceneManager::EndLodingImguiFunc()
{
	ImGuiLodingFunc = nullptr;
}

GameObject* SceneManager::FindObject(const wchar_t* name)
{
	GameObject* obj = nullptr;
	auto findIter = objectFindMap.find(name);
	if (findIter != objectFindMap.end())
	{
		int firstID = *(findIter->second.begin());
		obj = currScene->objectList[firstID].get();
	}
	return obj;
}

GameObject* SceneManager::GetObjectToID(unsigned int instanceID)
{
	GameObject* obj = nullptr;
	if (currScene && instanceID < currScene->objectList.size())
	{
		obj = currScene->objectList[instanceID].get();
	}
	return obj;
}

std::vector<GameObject*> SceneManager::FindObjects(const wchar_t* name)
{
	std::vector<GameObject*> objects;
	auto findIter = objectFindMap.find(name);
	if(findIter != objectFindMap.end())
	{
		for (auto index : findIter->second)
		{
			objects.push_back(currScene->objectList[index].get());
		}
	}
	return objects;
}

void SceneManager::FixedUpdateScene()
{
	currScene->FixedUpdate();
}

void SceneManager::UpdateScene()
{
	DXTKinputSystem.Update();
	currScene->Update();
}

void SceneManager::LateUpdateScene()
{
	currScene->LateUpdate();
}

void SceneManager::RenderScene()
{
	d3dRenderer.ClearRTV();
	if (ImGuiLodingFunc)
	{
		d3dRenderer.SetRTVdefault();
		Scene::ImGUIBegineDraw();
		ImGuiLodingFunc();
		Scene::ImGUIEndDraw();
		d3dRenderer.Present();
	}
	else if(Camera::GetMainCamera())
		currScene->Render();
}

void SceneManager::AddObjects()
{
	while (!currAddQueue.empty())
	{
		auto& obj = currAddQueue.front();
		AddObjectCurrScene(obj);
		obj->transform.UpdateTransform();
		obj->transform.ResetFlagUpdateWM();	
		currAddQueue.pop();
	}
}

void SceneManager::EraseObjects()
{
	if (!eraseSet.empty())
	{
		for (auto& obj : eraseSet)
		{
			EraseObject(EraseObjectFindMap(obj));
		}
		eraseSet.clear();

		auto& objList = currScene->objectList;
		while (!objList.empty() && objList.back() == nullptr)
		{
			objList.pop_back();
		}
	}
	if (!eraseComponentSet.empty())
	{
		for (auto& component : eraseComponentSet)
		{
			component->gameObject.EraseComponent(component);
		}
		eraseComponentSet.clear();
	}
}

void SceneManager::ChangeScene()
{
	if (nextScene && !ImGuiLodingFunc)
	{
		while (!nextAddQueue.empty())
		{
			auto& obj = nextAddQueue.front();
			AddObjectNextScene(obj);
			nextAddQueue.pop();
		}
		if (currScene)
		{
			std::erase_if(currScene->dontdestroyonloadList, [](std::weak_ptr<GameObject> ptr) {return ptr.expired(); });
			if (!currScene->dontdestroyonloadList.empty())
			{
				nextScene->dontdestroyonloadList = std::move(currScene->dontdestroyonloadList);
				for (auto& weakptr : nextScene->dontdestroyonloadList)
				{
					unsigned int id = weakptr.lock()->GetInstanceID();
					if (nextScene->objectList.size() <= id)
						nextScene->objectList.resize((size_t)id + 1);

					nextScene->objectList[id] = currScene->objectList[id];
				}
			}
			currScene.reset();
			MeshRender::ReloadShaderAll(); //유효한 메시 객체들 셰이더 다시 생성
		}
		currScene = std::move(nextScene);
		isStartScene = false;
		currScene->Start();	
		if (Camera::GetMainCamera() == nullptr)
		{
			auto mainCamera = std::make_shared<CameraObject>();
			mainCamera->SetMainCamera();
			mainCamera->Name = L"MainCamera";
			mainCamera->instanceID = instanceIDManager.getUniqueID();
			auto obj = std::static_pointer_cast<GameObject>(mainCamera);
			AddObjectCurrScene(obj);
		}
		isStartScene = true;
	}
	else if (EndGame)
	{
		WinGameApp::GameEnd();
		currScene.reset();
		nextScene.reset();
	}
}

void SceneManager::AddObjectCurrScene(std::shared_ptr<GameObject>& obj)
{
	unsigned int id = obj->GetInstanceID();
	objectFindMap[obj->Name].insert(id);
	if (id >= currScene->objectList.size())
	{
		currScene->objectList.resize((size_t)id + 1);
	}
	currScene->objectList[id] = obj;
}

void SceneManager::AddObjectNextScene(std::shared_ptr<GameObject>& obj)
{
	unsigned int id = obj->GetInstanceID();
	objectFindMap[obj->Name].insert(id);
	if (id >= nextScene->objectList.size())
	{
		nextScene->objectList.resize((size_t)id + 1);
	}
	nextScene->objectList[id] = obj;
}

void SceneManager::ChangeObjectName(unsigned int instanceID, const std::wstring& _pervName, const std::wstring& _newName)
{
	auto pervNameIter = objectFindMap.find(_pervName);
	if (pervNameIter == objectFindMap.end())
		return;

	objectFindMap[_newName].insert(instanceID);
	objectFindMap[_pervName].erase(instanceID);
}

void SceneManager::EraseObject(unsigned int id)
{
	currScene->objectList[id].reset(); //오브젝트 삭제
}

unsigned int SceneManager::EraseObjectFindMap(GameObject* obj)
{
	auto findIter = objectFindMap.find(obj->Name);
	if (findIter == objectFindMap.end())
		return -1;

	unsigned int index = obj->GetInstanceID();
	findIter->second.erase(obj->GetInstanceID()); //맵에서 삭제

	return index;
}


