#include "CameraObject.h"
#include <Manager/SceneManager.h>
#include <Utility/SerializedUtility.h>
#include <Component/Camera/CameraMoveHelper.h>

CameraObject::CameraObject()
{
	cam = &AddComponent<Camera>();
}

CameraObject::~CameraObject()
{


}

void CameraObject::Serialized(std::ofstream& ofs)
{
	using namespace Binary;
	bool isMainCam = false;
	if (cam == Camera::GetMainCamera())
	{
		isMainCam = true;
	}
	Write::data<bool>(ofs, isMainCam);

	bool isHelper = !!IsComponent<CameraMoveHelper>();
	Write::data<bool>(ofs, isHelper);
}

void CameraObject::Deserialized(std::ifstream& ifs)
{
	using namespace Binary;
	if (bool isMainCam = Read::data<bool>(ifs))
	{
		SetMainCamera();
	}
	if (bool isHelper = Read::data<bool>(ifs))
	{
		AddComponent<CameraMoveHelper>();
	}
}

void CameraObject::SetMainCamera()
{
	cam->SetMainCamera();
}

