#include "PBRBoneMeshRender.h"
#include <Light/PBRDirectionalLight.h>
#include <Manager/HLSLManager.h>

PBRBoneMeshRender::PBRBoneMeshRender()
{

}

void PBRBoneMeshRender::Start()
{
	SimpleBoneMeshRender::Start();

    //BRDF LookUp Table Sampler
    samplerState.resize(3);
    D3D11_SAMPLER_DESC SamplerDesc = {};
    SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    SamplerDesc.MinLOD = 0;
    SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    samplerState.SetSamplerState(1, SamplerDesc);

    //Shadow Sampler
    SamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    SamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerState.SetSamplerState(2, SamplerDesc);

    PBRMeshObject* meshObj = dynamic_cast<PBRMeshObject*>(&gameObject);
    if (meshObj)
    {
        int index = constBuffer.CreatePSConstantBuffers<cb_PBRDirectionalLight>(DirectionalLight::DirectionalLights_key);
        std::string materialKey = meshObj->GetNameToString();
        index = constBuffer.CreatePSConstantBuffers<cb_PBRMaterial>(materialKey.c_str());
        {
            using namespace std::string_literals;
            std::wstring vertexPath(HLSLManager::EngineShaderPath + L"VertexSkinningShader.hlsl"s);
            SetVertexShader(vertexPath.c_str());

            std::wstring pixelPath(HLSLManager::EngineShaderPath + L"PBROpaquePS.hlsl"s);
            SetPixelShader(pixelPath.c_str());
        }
    }
}
