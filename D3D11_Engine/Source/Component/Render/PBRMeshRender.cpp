#include "PBRMeshRender.h"
#include <Light/PBRDirectionalLight.h>
#include <Manager/HLSLManager.h>

PBRMeshRender::PBRMeshRender()
{
    
}

void PBRMeshRender::Start()
{
    meshObject = dynamic_cast<PBRMeshObject*>(&gameObject);
	if (!meshObject)
	{
		Debug_wprintf(L"Warning : PBRMeshRender can only be added to PBRMeshObject.\n");
		GameObject::DestroyComponent(this);
		return;
	}
	SimpleMeshRender::Start();

	//BRDF LookUp Table Sampler
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	materialAsset.SetSamplerState(sampDesc, 1);

	//Shadow Sampler
	sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	materialAsset.SetSamplerState(sampDesc, 2);

	{
		using namespace std::string_literals;
		std::wstring vertexPath(HLSLManager::EngineShaderPath + L"VertexShader.hlsl"s);
		SetVS(vertexPath.c_str());

		std::wstring pixelPath(HLSLManager::EngineShaderPath + L"PBROpaquePS.hlsl"s);
		SetPS(pixelPath.c_str());
	}
}

void PBRMeshRender::UpdateMeshDrawCommand()
{
	//머터리얼 업데이트
	material.Set(meshObject->Material);
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
