#pragma once
#include <GameObject/Base/GameObject.h>
#include <Component/Camera/Camera.h>

class CameraObject : public GameObject
{
	SERIALIZED_OBJECT(CameraObject)
public:
	CameraObject();
	virtual ~CameraObject() override;
	virtual void Serialized(std::ofstream& ofs);
	virtual void Deserialized(std::ifstream& ifs);
	virtual void InspectorImguiDraw();
public:
	void SetMainCamera();
	
private:
	Camera* cam = nullptr;
};