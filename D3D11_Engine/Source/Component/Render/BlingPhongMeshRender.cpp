#include "BlingPhongMeshRender.h"
#include <Light/PBRDirectionalLight.h>
#include <Manager/HLSLManager.h>

void BlingPhongMeshRender::Start()
{
	meshObj = dynamic_cast<BlingPhongMeshObject*>(&gameObject);
	if (!meshObj)
	{
		Debug_wprintf(L"Warning : BlingPhongMeshRender can only be added to BlingPhongMeshObject.\n");
		GameObject::DestroyComponent(this);
		return;
	}
	SimpleMeshRender::Start();
	isForward = true;

	{
		using namespace std::string_literals;
		std::wstring vertexPath(HLSLManager::EngineShaderPath + L"VertexShader.hlsl"s);
		SetVS(vertexPath.c_str());

		std::wstring pixelPath(HLSLManager::EngineShaderPath + L"BlingPhongPixelShader.hlsl"s);
		SetPS(pixelPath.c_str());
	}
}

void BlingPhongMeshRender::UpdateMeshDrawCommand()
{
	//머터리얼 업데이트
	material.Set(meshObj->Material);
	meshDrawCommand.materialData.shaderResources.push_back(
		Binadble
		{
			.shaderType = EShaderType::Pixel,
			.bindableType = EShaderBindable::ConstantBuffer,
			.slot = 4,
			.bind = (ID3D11Buffer*)material
		}
	);
}
