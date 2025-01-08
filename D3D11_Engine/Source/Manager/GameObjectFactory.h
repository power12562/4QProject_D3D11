#pragma once
#include <Core/TSingleton.h>
#include <Manager/SceneManager.h>
#include <map>
#include <string>
#include <functional>
#include <Core/StaticBlockMemoryPool.h>

/*오브젝트 클래스 선언시 다음 매크로 포함*/
#define SERIALIZED_OBJECT(TypeName)	\
friend class GameObjectFactory;	\
inline static bool TypeName##FactoryInit = GameObjectFactory::AddNewObjectFuntion<TypeName>(); \

class GameObject;
class GameObjectFactory;
extern GameObjectFactory& gameObjectFactory;

class GameObjectFactory : public TSingleton<GameObjectFactory>
{
	friend class TSingleton;
	friend class D3D11_GameApp;
	inline static std::unique_ptr<std::map<std::string, std::function<GameObject* (const wchar_t* name)>>> newGameObjectFuncMap;
	inline static size_t MaxGameObjectClassSize = 0;
	inline static std::string MaxSizeGameObjectClassName;
public:
	//게임 오브젝트 커스텀 딜리터
	static void GameObjectDeleter(GameObject* pObj);

	template<typename T>
	static bool AddNewObjectFuntion()
	{
		static_assert(std::is_base_of_v<GameObject, T>, "T is not gameObject");
		std::string key = typeid(T).name();

		auto func = [](const wchar_t* name) { return NewGameObject<T>(name); };

		if (!newGameObjectFuncMap)
		{
			newGameObjectFuncMap.reset(new std::map<std::string, std::function<GameObject* (const wchar_t* name)>>);
		}
		static std::map<std::string, std::function<GameObject* (const wchar_t* name)>>& objectFuncMap = *newGameObjectFuncMap;
		objectFuncMap[key] = func;

		if (MaxGameObjectClassSize < sizeof(T))
		{
			MaxGameObjectClassSize = sizeof(T);
			MaxSizeGameObjectClassName = key;
		}
		return true;
	}

private:
	GameObjectFactory() = default;
	~GameObjectFactory() override;
	
private:
	StaticBlockMemoryPool gameObjectMemoryPool;
public:
	void InitializeMemoryPool();
	void UninitializeMemoryPool();

	void* GameObjectAlloc(size_t id);
	std::function<GameObject*(const wchar_t* name)>& NewGameObjectToKey(const char* key);

	void SerializedObject(GameObject* object, const wchar_t* WritePath, bool isOverride = false);
	GameObject* DeserializedObject(const wchar_t* ReadPath);
private:
	void Serialized(GameObject* object, std::ofstream& ofs, size_t level);
	GameObject* Deserialized(std::ifstream& ifs);
};

