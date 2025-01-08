#pragma once
#include <GameObject/Base/GameObject.h>
#include <Component/Camera/Camera.h>

class CameraObject : public GameObject
{
	SERIALIZED_OBJECT(CameraObject)
public:
	CameraObject();
	virtual ~CameraObject() override;

public:
	void SetMainCamera();
	
private:
	Camera* cam = nullptr;
};