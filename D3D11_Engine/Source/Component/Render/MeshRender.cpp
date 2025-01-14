#include "MeshRender.h"
#include <Manager/ResourceManager.h>
#include <Manager/HLSLManager.h>
#include <Utility/MemoryUtility.h>
#include <ranges>
#include <Material/PBRMaterial.h>

using namespace Utility;

void MeshRender::ReloadShaderAll()
{
	hlslManager.ClearSharingShader();
	for (auto& item : instanceList)
	{
		
	}
}

MeshRender::MeshRender()
{

}

MeshRender::~MeshRender()
{

}

void MeshRender::Render()
{
	transformBuffer.Set(transform.GetWM());

}

void MeshRender::SetMeshResource(const wchar_t* path)
{
	if (MeshID < 0)
	{
		__debugbreak();
		return;
	}

	using namespace utfConvert;
	if ((ID3D11Buffer*)meshDrawCommand.meshData.vertexBuffer == nullptr)
	{
		//고유의 경로 + index
		sharedMeshData = GetResourceManager<MeshData>().GetResource(path, MeshID);
		meshDrawCommand.meshData = *sharedMeshData;
	}
}

