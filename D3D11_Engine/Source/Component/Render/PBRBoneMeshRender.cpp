#include "PBRBoneMeshRender.h"
#include <Light/PBRDirectionalLight.h>
#include <Manager/HLSLManager.h>

PBRBoneMeshRender::PBRBoneMeshRender()
{

}

void PBRBoneMeshRender::Start()
{
    meshObject = dynamic_cast<PBRMeshObject*>(&gameObject);
    if (!meshObject)
    {
        Debug_wprintf(L"Warning : PBRBoneMeshRender can only be added to PBRMeshObject.\n");
        GameObject::DestroyComponent(this);
        return;
    }

	SimpleBoneMeshRender::Start();

    //BRDF LookUp Table Sampler
    D3D11_SAMPLER_DESC SamplerDesc = {};
    SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    SamplerDesc.MinLOD = 0;
    SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    materialAsset.SetSamplerState(SamplerDesc, 1);

    //Shadow Sampler
    SamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    SamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    materialAsset.SetSamplerState(SamplerDesc, 2);

	{
		using namespace std::string_literals;
		std::wstring vertexPath(HLSLManager::EngineShaderPath + L"VertexSkinningShader.hlsl"s);
		SetVS(vertexPath.c_str());

		std::wstring pixelPath(HLSLManager::EngineShaderPath + L"PBROpaquePS.hlsl"s);
		SetPS(pixelPath.c_str());
	}
}

void PBRBoneMeshRender::UpdateMeshDrawCommand()
{
    SimpleBoneMeshRender::UpdateMeshDrawCommand();

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
