#include "CameraObject.h"
#include <Manager/SceneManager.h>

CameraObject::CameraObject()
{
	cam = &AddComponent<Camera>();
}

CameraObject::~CameraObject()
{


}

void CameraObject::SetMainCamera()
{
	cam->SetMainCamera();
}

