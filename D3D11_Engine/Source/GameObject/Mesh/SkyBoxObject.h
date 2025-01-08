#pragma once
#include <GameObject/Base/GameObject.h>

class SkyBoxRender;
class SkyBoxObject : public GameObject
{
	SERIALIZED_OBJECT(SkyBoxObject)
public:
	SkyBoxObject();
	virtual ~SkyBoxObject() override = default;

	SkyBoxRender& skyBoxRender;
private:
	using GameObject::transform;
};