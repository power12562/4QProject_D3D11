#include "Component.h"
#include <GameObject\Base\GameObject.h>
#include <Utility/Console.h>

void Component::SetOwner(GameObject* gameObject)
{
	this->_gameObject = gameObject;	
}

Component::Component()
{
	
}

Component::~Component()
{
	//Debug_printf("ÄÄÆ÷³ÍÆ® ¼Ò¸ê\n");
}

GameObject& Component::GetGameObject() const
{
	return *_gameObject;
}

Transform& Component::GetTransform() const
{
	return gameObject.transform;
}

Transform& Component::SetTransform(const Transform& tr)
{
	gameObject.transform = tr;
	return gameObject.transform;
}

