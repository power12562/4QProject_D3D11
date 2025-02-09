#include "GameObjectFactory.h"
#include <GameObject/Base/GameObject.h>
#include <framework.h>
#include <Utility/SerializedUtility.h>
#include <Component/Render/SimpleBoneMeshRender.h>
#include <ranges>
#include <algorithm>

GameObjectFactory& gameObjectFactory = GameObjectFactory::GetInstance();

GameObjectFactory::~GameObjectFactory()
{
	gameObjectMemoryPool.Uninitialize();
}

void GameObjectFactory::GameObjectDeleter(GameObject* pObj)
{
	unsigned int id = pObj->GetInstanceID();
	pObj->~GameObject();
	gameObjectFactory.gameObjectMemoryPool.Free(id);
}

void GameObjectFactory::InitializeMemoryPool()
{
	gameObjectMemoryPool.Initialize(MaxGameObjectClassSize, MaxGameObjectClassSize * 500);
}

void GameObjectFactory::UninitializeMemoryPool()
{
	gameObjectMemoryPool.Uninitialize();
}

void GameObjectFactory::CompactObjectMemoryPool()
{
	gameObjectMemoryPool.CompactMemoryPage();
}

void* GameObjectFactory::GameObjectAlloc(size_t id)
{
	return gameObjectMemoryPool.Allocate(id);
}

std::function<GameObject*(const wchar_t* name)>& GameObjectFactory::NewGameObjectToKey(const char* key)
{
	auto findIter = newGameObjectFuncMap->find(key);
	if (findIter != newGameObjectFuncMap->end())
	{
		return findIter->second;
	}
	__debugbreak(); //존재하지 않는 키.
	throw std::runtime_error("Key not found in the map");
}

void GameObjectFactory::SerializedScene(Scene* scene, const wchar_t* WritePath, bool isOverride)
{
	if (scene == nullptr)
	{
		__debugbreak(); //scene is a nullptr
		return;
	}

	std::filesystem::path path(WritePath);
	std::filesystem::path extension = path.extension();
	if (extension != L".Scene")
	{
		if (extension != L"")
		{
			path.replace_extension(L".Scene");
		}
		else
		{
			if (!Utility::IsPathDirectory(path))
			{
				path += L"/";
			}
			path += scene->GetSceneName();
			path += L".Scene";
		}
	}

	if (!std::filesystem::exists(path))
	{
		if (!std::filesystem::exists(path.parent_path()))
		{
			std::filesystem::create_directories(path.parent_path());
		}
	}
	else if (!isOverride)
	{
		int result = MessageBox(
			NULL,
			L"파일이 존재합니다. 덮어쓰시겠습니까?",
			path.c_str(),
			MB_OKCANCEL | MB_ICONQUESTION
		);
		if (result == IDCANCEL) {
			return;
		}
	}

	ObjectList objList = sceneManager.GetObjectList();
	auto parentObjects = objList | std::views::filter([](GameObject* obj) { return !obj->transform.Parent;});
	size_t objCount = std::ranges::distance(parentObjects);
	std::ofstream ofs(path.c_str(), std::ios::binary | std::ios::trunc);
	for (const auto& item : parentObjects)
	{
		Serialized(item, ofs, 0);
	}
	ofs.close();
}

void GameObjectFactory::DeserializedScene(Scene* scene, const wchar_t* ReadPath)
{
	std::filesystem::path path(ReadPath);
	if (!std::filesystem::exists(path))
	{
		MessageBox(NULL, L"파일이 존재하지 않습니다.", ReadPath, MB_OK);
		return;
	}
	if (path.extension() != L".Scene")
	{
		MessageBox(NULL, L"Scene 파일이 아닙니다.", ReadPath, MB_OK);
		return;
	}

	std::ifstream ifs(path.c_str(), std::ios::binary);
	Deserialized(ifs, scene);
	ifs.close();
}

void GameObjectFactory::SerializedObject(GameObject* object, const wchar_t* WritePath, bool isOverride)
{
	if (object == nullptr)
	{
		__debugbreak(); //존재하지 않는 오브젝트
		return;
	}

	if (newGameObjectFuncMap->find(typeid(*object).name()) == newGameObjectFuncMap->end())
	{
		MessageBox(NULL, L"Is not Serialized Object", object->Name.c_str(), MB_OK);
		return;
	}
	std::filesystem::path path(WritePath);
	std::filesystem::path extension = path.extension();
	if (extension != L".GameObject")
	{
		if (extension != L"")
		{
			path.replace_extension(L".GameObject");
		}
		else
		{
			if (!Utility::IsPathDirectory(path))
			{
				path += L"/";
			}
			path += object->GetName();
			path += L".GameObject";
		}
	}

	if (!std::filesystem::exists(path))
	{
		if (!std::filesystem::exists(path.parent_path()))
		{
			std::filesystem::create_directories(path.parent_path());
		}
	}
	else if(!isOverride)
	{
		int result = MessageBox(
			NULL,                      
			L"파일이 존재합니다. 덮어쓰시겠습니까?", 
			path.c_str(),
			MB_OKCANCEL | MB_ICONQUESTION 
		);
		if (result == IDCANCEL) {
			return;
		}
	}	

	std::ofstream ofs(path.c_str(), std::ios::binary | std::ios::trunc);
	Serialized(object, ofs, 0);
	ofs.close();
}

GameObject* GameObjectFactory::DeserializedObject(const wchar_t* ReadPath)
{
	std::filesystem::path path(ReadPath);
	if (!std::filesystem::exists(path))
	{
		MessageBox(NULL, L"파일이 존재하지 않습니다.", ReadPath, MB_OK);
		return nullptr;
	}
	if (path.extension() != L".GameObject")
	{
		MessageBox(NULL, L"GameObject 파일이 아닙니다.", ReadPath, MB_OK);
		return nullptr;
	}

	std::ifstream ifs(path.c_str(), std::ios::binary);
	GameObject* obj = Deserialized(ifs);
	ifs.close();
	return obj;
}

void GameObjectFactory::Serialized(GameObject* object, std::ofstream& ofs, size_t level)
{
	using namespace Binary;
	const char* typeName = typeid(*object).name();
	Write::string(ofs, typeName);
	const wchar_t* objName = object->Name.c_str();
	Write::wstring(ofs, objName);
	Write::data(ofs, object->Active);
	Write::BoundingBox(ofs, object->Bounds);
	object->Serialized(ofs);

	//Components
	for (auto& component : object->componentList)
	{
		component->Serialized(ofs);
	}

	//Transform
	Write::data<size_t>(ofs, level);
	bool isParent = level > 0;
	if (!isParent)
	{
		Write::Vector3(ofs, object->transform.position);
		Write::Quaternion(ofs, object->transform.rotation);
		Write::Vector3(ofs, object->transform.scale);
	}
	else
	{
		Write::Vector3(ofs, object->transform.localPosition);
		Write::Quaternion(ofs, object->transform.localRotation);
		Write::Vector3(ofs, object->transform.localScale);
	}	

	//TransformAnimation
	TransformAnimation* animation = object->IsComponent<TransformAnimation>();
	bool isTransformAnimation = animation != nullptr;
	Write::data(ofs, isTransformAnimation);
	if (animation)
		animation->SerializedAnimation(ofs);

	//childs
	for (size_t i = 0; i < object->transform.GetChildCount(); i++)
	{
		if(GameObject* chidObject = &object->transform.GetChild(i)->gameObject)
			Serialized(chidObject, ofs, level + 1);
	}
}

GameObject* GameObjectFactory::Deserialized(std::ifstream& ifs, Scene* scene)
{
	static std::map<size_t, Transform*> objectTreeMap;
	std::vector<TransformAnimation*> transformAnimationVec;
	GameObject* root = nullptr;
	using namespace Binary;
	while (true)
	{
		std::string typeName = Read::string(ifs);
		if (ifs.eof())
		{
			objectTreeMap.clear();
			for (auto& animation : transformAnimationVec)
			{
				animation->AddChildrenToTargets();
			}
			SimpleBoneMeshRender::EndDeserialized();
			return root;
		}
		std::wstring objName = Read::wstring(ifs);
		GameObject* object = NewGameObjectToKey(typeName.c_str())(objName.c_str());
		object->Active = Read::data<bool>(ifs);
		object->Bounds = Read::BoundingBox(ifs);
		object->Deserialized(ifs);
		//Components
		for (auto& component : object->componentList)
		{
			component->Deserialized(ifs);
		}
		//Transform
		size_t myLevel = Read::data<size_t>(ifs);
		objectTreeMap[myLevel] = &object->transform;
		bool isParent = myLevel > 0;
		if (isParent)
		{
			auto find = objectTreeMap.find(myLevel - 1); //마지막으로 생성된 내 상위 객체를 찾는다
			if (find != objectTreeMap.end())
				object->transform.SetParent(find->second); //찾으면 부모로 설정
		}
		if (!isParent)
		{
			object->transform.position = Read::Vector3(ifs);
			object->transform.rotation = Read::Quaternion(ifs);
			object->transform.scale	   = Read::Vector3(ifs);
			root = object;

			if (scene)
			{
				scene->loadScenesMap[scene->GetSceneName()].emplace_back(object->GetWeakPtr());
			}
		}
		else
		{
			object->transform.localPosition = Read::Vector3(ifs);
			object->transform.localRotation = Read::Quaternion(ifs);
			object->transform.localScale	= Read::Vector3(ifs);
		}

		//TransformAnimation
		bool isTransformAnimation = Read::data<bool>(ifs);
		if (isTransformAnimation)
		{
			TransformAnimation* animation = &object->AddComponent<TransformAnimation>();
			animation->DeserializedAnimation(ifs);
			transformAnimationVec.push_back(animation);
		}
	}
}
 