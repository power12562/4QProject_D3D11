#include "SphereObject.h"
#include <Light/PBRDirectionalLight.h>
#include <Manager/HLSLManager.h>
#include <Utility/AssimpUtility.h>
#include <Component/Render/PBRMeshRender.h>
#include <filesystem>

void SphereObject::Start()
{
	PBRMeshObject::Start();
	{
		using namespace std::string_literals;
		static bool isInit = false;

		if (isInit)
		{
			std::filesystem::path objectPath(Utility::GetTempResourcePath(L"sphere.fbx"));
			gameObjectFactory.DeserializedObject(objectPath.c_str())->transform.SetParent(this->transform);;
		}		
		else
		{
			std::wstring fbxPath(HLSLManager::EngineShaderPath + L"sphere.fbx"s);
			Utility::LoadFBX(fbxPath.c_str(), false, SURFACE_TYPE::PBR)->transform.SetParent(this->transform);
			isInit = true;
		}
	}

	for (unsigned int i = 0; i < transform.GetChildCount(); i++)
	{
		for (unsigned int j = 0; j < transform.GetChild(i)->GetChildCount(); j++)
		{
			SphereMeshRender = transform.GetChild(i)->GetChild(j)->gameObject.IsComponent<PBRMeshRender>();
			if (SphereMeshRender)
			{
				transform.scale = 0.1f;
				break;
			}
		}
	}
}
