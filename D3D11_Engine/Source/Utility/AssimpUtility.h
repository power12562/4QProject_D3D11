#pragma once
#include <memory>
#include <Math\AssimpMath.h>
#include <Core/Transform.h>
#include <functional>
#include <filesystem>

class MeshRender;
class GameObject;

enum class SURFACE_TYPE
{
	BlingPhong,
	PBR,
	NONE
};

namespace Utility
{
	bool ParseFileName(aiString& str);

	GameObject* LoadFBX(const wchar_t* path, 
		std::function<void(MeshRender*)> initMesh,
		bool isStatic, 
		SURFACE_TYPE surface);

	GameObject* LoadFBX(const wchar_t* path,
		bool isStatic,
		SURFACE_TYPE surface);

	void LoadFBXResource(const wchar_t* path,
		std::function<void(MeshRender*)> initMesh,
		bool isStatic, 
		SURFACE_TYPE surface);

	void LoadFBXResource(const wchar_t* path,
		bool isStatic,
		SURFACE_TYPE surface);

	std::filesystem::path GetTempResourcePath(const wchar_t* flieName);
}
