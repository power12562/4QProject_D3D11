#pragma once
#include <Core/Transform.h>
#include <Utility/Console.h>
#include <memory>
#include <string>
#include <Manager/GameObjectFactory.h>
#include <Utility/ExceptionUtility.h>

#pragma warning( disable : 4267)

class Component;
class TransformAnimation;
class RenderComponent;
class GameObject
{		 
	SERIALIZED_OBJECT(GameObject)
	friend class Scene;
	friend class SceneManager;
	friend class D3DRenderer;
public:
	GameObject();
	/*이름, 인스턴스 아이디 부여 후 호출되는 함수.*/
	virtual void Start() {};
	virtual ~GameObject();

	/** 추가적으로 직렬화할 데이터 필요시 오버라이딩*/
	virtual void Serialized(std::ofstream& ofs) {};
	/** 추가적으로 직렬화할 데이터 필요시 오버라이딩*/
	virtual void Deserialized(std::ifstream& ifs) {};
public:
	static void Destroy(GameObject& obj);
	static void Destroy(GameObject* obj);
	static void DontDestroyOnLoad(GameObject& obj);
	static void DontDestroyOnLoad(GameObject* obj);
	static GameObject* Find(const wchar_t* name);
	template<typename ObjectType>
	inline static ObjectType* NewGameObject(const wchar_t* name);

public:
	unsigned int GetInstanceID() const { return instanceID; }
	const std::wstring& GetName() const { return name; }
	const std::wstring& SetName(const wchar_t* _name);
	std::string GetNameToString() const;
private:
	std::wstring name;
	unsigned int instanceID = -1;

public:
	Transform transform;
	bool Active = true;

public:
	/*오브젝트 이름. (중복 가능)*/
	_declspec (property(get = GetName, put = SetName)) std::wstring& Name;

	/*컴포넌트 추가*/
	template <typename T>
	T& AddComponent();

	/*TransformAnimation은 하나만 존재 가능.*/
	template <>
	TransformAnimation& AddComponent();

	/*컴포넌트 가져오기*/
	template <typename T>
	T& GetComponent();

	/*컴포넌트 가져오기. (없으면 nullptr 반환)*/
	template <typename T>
	T* IsComponent();

	/*컴포넌트 주소로 인덱스 확인하기. 없으면 -1 반환*/
	int GetComponentIndex(Component* findComponent);

	/*인덱스로 컴포넌트 가져오기. 파라미터로 캐스팅할 컴포넌트 타입을 전달.*/
	template <typename T>
	T* GetComponentAtIndex(int index);

	/*이 오브젝트의 컴포넌트 개수*/
	int GetComponentCount() { return componentList.size(); }

	void DestroyComponent(Component& component);
	void DestroyComponent(Component* component);

	DirectX::BoundingOrientedBox GetOBBToWorld() const;
	DirectX::BoundingBox GetBBToWorld() const;

	//오브젝트 Bounds 초기 center 값은 Mathf::FLOAT_MAX로 초기화 됩니다.
	DirectX::BoundingBox Bounds;

	/*카메라 컬링 여부*/
	inline bool IsCameraCulling() const { return isCulling; }
private:
	void FixedUpdate();
	void Update();
	void LateUpdate();
	void Render();
private:
	void EraseComponent(Component* component);

private:
	std::vector<std::unique_ptr<Component>> componentList;
	std::vector<RenderComponent*> renderList;
public:
	std::weak_ptr<GameObject> GetWeakPtr() { return myptr; }
private:
	std::weak_ptr<GameObject> myptr;
	bool isCulling = false; //이번 프레임 카메라 컬링 여부
	bool checkActive = true;
	inline bool CheckActive() const { return checkActive != Active; }
	void UpdateChildActive(Transform* rootTransform);
};

template<typename T>
inline T& GameObject::AddComponent()
{
	static_assert(std::is_base_of_v<Component, T>, "is not Component");

	T* nComponent = new T;
	nComponent->SetOwner(this);
	nComponent->index = componentList.size();
	nComponent->Start();
	componentList.emplace_back(nComponent);	
	if constexpr (std::is_base_of_v<RenderComponent, T>)
	{
		renderList.push_back(nComponent);
	}
	return *nComponent;
}

template<typename T>
inline T& GameObject::GetComponent()
{
	static_assert(std::is_base_of_v<Component, T>, "is not Component");

	for (auto& component : componentList)
	{
		if (typeid(*component) == typeid(T))
		{
			return dynamic_cast<T&>(*component);
		}
	}
	__debugbreak(); //예외) 존재하지 않는 컴포넌트
	throw_GameObject("Exception : Component does not exist", this);
}

template<typename T>
inline T* GameObject::IsComponent()
{
	static_assert(std::is_base_of_v<Component, T>, "is not Component");

	for (auto& component : componentList)
	{
		if (typeid(*component) == typeid(T))
		{
			return dynamic_cast<T*>(component.get());
		}
	}
	return nullptr;
}

template<typename T>
inline T* GameObject::GetComponentAtIndex(int index)
{
	static_assert(std::is_base_of_v<Component, T>, "is not Component");

	if (0 <= index && index < componentList.size())
	{
		T* component = dynamic_cast<T*>(componentList[index].get());
		return component;
	}
	Debug_printf("warrnig : GetComponentAtIndex(int index), index is out of range!\n");
	return nullptr;
}

template<typename ObjectType = GameObject>
ObjectType* NewGameObject(const wchar_t* name)
{
	return GameObject::NewGameObject<ObjectType>(name);
}

#include <Manager/InstanceIDManager.h>
template<typename ObjectType>
inline ObjectType* GameObject::NewGameObject(const wchar_t* name)
{
	static_assert(std::is_base_of_v<GameObject, ObjectType>, "is not gameObject");

	unsigned int id = instanceIDManager.getUniqueID();
	void* pointer = gameObjectFactory.GameObjectAlloc(id);
	if (pointer)
	{
		ObjectType* pObject = reinterpret_cast<ObjectType*>(pointer);
		std::shared_ptr<ObjectType> newObject(pObject, GameObjectFactory::GameObjectDeleter);
		std::shared_ptr<GameObject> baseObject = std::static_pointer_cast<GameObject>(newObject);

		new(pObject)ObjectType();
		baseObject->Name = name;
		baseObject->instanceID = id;
		baseObject->myptr = baseObject;
		baseObject->Start();

		sceneManager.AddGameObject(baseObject);
		return newObject.get();
	}
	else
	{
		__debugbreak();
		return nullptr;
	}
}