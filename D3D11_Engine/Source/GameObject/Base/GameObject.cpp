#include "GameObject.h"	   
#include <Utility/Console.h>

#include <Component/Animation/TransformAnimation.h>
#include <Component/Camera/Camera.h>
#include <Component/Base/RenderComponent.h>
#include <D3DCore/D3DRenderer.h>
#include <Manager/SceneManager.h>
#include <Manager/InstanceIDManager.h>
#include <D3DCore/D3DConstBuffer.h>
#include <Utility/utfConvert.h>
#include <Math/Mathf.h>

void GameObject::Destroy(GameObject& obj)
{
	sceneManager.DestroyObject(obj);
}

void GameObject::Destroy(GameObject* obj)
{
	sceneManager.DestroyObject(obj);
}

void GameObject::DontDestroyOnLoad(GameObject& obj)
{
	sceneManager.DontDestroyOnLoad(obj);
}

void GameObject::DontDestroyOnLoad(GameObject* obj)
{
	sceneManager.DontDestroyOnLoad(obj);
}

GameObject* GameObject::Find(const wchar_t* name)
{
	return sceneManager.FindObject(name);
}

GameObject::GameObject()
{
	transform._gameObject = this;	 
	Bounds.Center = { Mathf::FLOAT_MAX, Mathf::FLOAT_MAX, Mathf::FLOAT_MAX };
}

GameObject::~GameObject()
{
	//Debug_printf("%s, destroy\n", Name);
	if (Scene::GuizmoSetting.SelectObject == this)
	{
		Scene::GuizmoSetting.SelectObject = nullptr;
	}
	instanceIDManager.returnID(instanceID);
	sceneManager.EraseObjectFindMap(this);
}

template <>
TransformAnimation& GameObject::AddComponent()
{
	GameObject* obj = transform.rootParent ? &transform.rootParent->gameObject : this;
	if (!obj->IsComponent<TransformAnimation>())
	{
		TransformAnimation* nComponent = new TransformAnimation;
		nComponent->SetOwner(obj);
		nComponent->index = componentList.size();
		nComponent->Start();
		componentList.emplace_back(nComponent);
		return *nComponent;
	}
	else
	{
		__debugbreak();
		throw_GameObject("Error : Only one TransformAnimation component can be used.", this);
	}
}

int GameObject::GetComponentIndex(Component* findComponent)
{
	for (size_t i = 0; i < componentList.size(); i++)
	{
		if (componentList[i].get() == findComponent)
		{
			return i;
		}
	}
	return -1;
}

void GameObject::DestroyComponent(Component& component)
{
	DestroyComponent(&component);
}

void GameObject::DestroyComponent(Component* component)
{
	sceneManager.eraseComponentSet.insert(component);
}

DirectX::BoundingOrientedBox GameObject::GetOBBToWorld() const
{
	BoundingOrientedBox boundingBoxOut;
	boundingBoxOut.CreateFromBoundingBox(boundingBoxOut, Bounds);
	boundingBoxOut.Transform(boundingBoxOut, transform.GetWM());

	return boundingBoxOut;
}

DirectX::BoundingBox GameObject::GetBBToWorld() const
{
	//BB
	BoundingBox boundingBoxOut = Bounds;
	boundingBoxOut.Transform(boundingBoxOut, transform.GetWM());
	return boundingBoxOut;
}

void GameObject::FixedUpdate()
{
	for (auto& component : componentList)
	{
		if(component->Enable)
			component->FixedUpdate();
	}
}

void GameObject::Update()
{
	for (auto& component : componentList)
	{
		if (component->Enable)
			component->Update();
	}
}

void GameObject::LateUpdate()
{
	for (auto& component : componentList)
	{
		if (component->Enable)
			component->LateUpdate();
	}
}

void GameObject::Render()
{	
	for (auto& component : renderList)
	{
		if (component->Enable)
			component->Render();
	}
}

void GameObject::EraseComponent(Component* component)
{
	int size = (int)componentList.size();
	int comIndex = 0;
	for (auto& com : componentList)
	{
		if (typeid(*com) == typeid(*component))
			break;

		++comIndex;
	}
	int renderIndex = 0;
	if (comIndex < size)
	{
		size = renderList.size();
		for (auto& com : renderList)
		{
			if (typeid(*com) == typeid(*component))
				break;

			renderIndex++;
		}
		componentList.erase((componentList.begin() + comIndex));
		if (renderIndex < size)
			renderList.erase((renderList.begin() + renderIndex));
	}
}

void GameObject::UpdateChildActive(Transform* rootObject)
{
	for (auto& child : rootObject->childList)
	{
		child->gameObject.Active = rootObject->gameObject.Active;
		child->gameObject.checkActive = rootObject->gameObject.checkActive;
		if (!child->childList.empty())
			UpdateChildActive(child);
	}
}

const std::wstring& GameObject::SetName(const wchar_t* _name)
{
	if (wcscmp(name.c_str(), _name) == 0)
		return name;

	sceneManager.ChangeObjectName(this->instanceID, name, _name);
	name = _name;
	return name;
}

std::string GameObject::GetNameToString() const
{
	return utfConvert::wstring_to_utf8(GetName());
}