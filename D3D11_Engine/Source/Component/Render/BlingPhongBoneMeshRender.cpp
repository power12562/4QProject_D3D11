#include "BlingPhongBoneMeshRender.h"
#include <Light/PBRDirectionalLight.h>
#include <Manager/HLSLManager.h>

void BlingPhongBoneMeshRender::Start()
{
	meshObj = dynamic_cast<BlingPhongMeshObject*>(&gameObject);
	if (!meshObj)
	{
		Debug_wprintf(L"Warning : BlingPhongBoneMeshRender can only be added to BlingPhongMeshObject.\n");
		GameObject::DestroyComponent(this);
		return;
	}

	SimpleBoneMeshRender::Start();

	{
		using namespace std::string_literals;
		std::wstring vertexPath(HLSLManager::EngineShaderPath + L"VertexSkinningShader.hlsl"s);
		SetVS(vertexPath.c_str());

		std::wstring pixelPath(HLSLManager::EngineShaderPath + L"BlingPhongPixelShader.hlsl"s);
		SetPS(pixelPath.c_str());
	}
}

void BlingPhongBoneMeshRender::UpdateMeshDrawCommand()
{
	SimpleBoneMeshRender::UpdateMeshDrawCommand();
	//���͸��� ������Ʈ
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
