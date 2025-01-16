#include "SkyBoxRender.h"
#include <Manager/HLSLManager.h>

SkyBoxRender* SkyBoxRender::GetMainSkyBox()
{
	if (mainSkyBox && mainSkyBox->Enable && mainSkyBox->gameObject.Active)
	{
        return mainSkyBox->materialAsset.GetTexturesV2().empty() ? nullptr : mainSkyBox;
	}
    return nullptr;
}

SkyBoxRender::~SkyBoxRender()
{
    if (mainSkyBox == this)
    {
        mainSkyBox = nullptr;
    }
}

void SkyBoxRender::Start()
{
    if (mainSkyBox)
    {
        Debug_printf("Warning : There can only be one skybox.\n"); //��ī�� �ڽ��� �ϳ��� ���� �����մϴ�.
        GameObject::Destroy(this->gameObject);
        return;
    }

    // Front face
    vertices.emplace_back(-1.0f, -1.0f, -1.0f, 1.0f);
    vertices.emplace_back(1.0f, -1.0f, -1.0f, 1.0f);
    vertices.emplace_back(1.0f, 1.0f, -1.0f, 1.0f);
    vertices.emplace_back(-1.0f, 1.0f, -1.0f, 1.0f);

    // Back face
    vertices.emplace_back(-1.0f, -1.0f, 1.0f, 1.0f);
    vertices.emplace_back(1.0f, -1.0f, 1.0f, 1.0f);
    vertices.emplace_back(1.0f, 1.0f, 1.0f, 1.0f);
    vertices.emplace_back(-1.0f, 1.0f, 1.0f, 1.0f);

    // Indices for the cube (two triangles per face)
    // Front face
    indices.emplace_back(0); indices.emplace_back(1); indices.emplace_back(2);
    indices.emplace_back(0); indices.emplace_back(2); indices.emplace_back(3);

    // Back face
    indices.emplace_back(4); indices.emplace_back(6); indices.emplace_back(5);
    indices.emplace_back(4); indices.emplace_back(7); indices.emplace_back(6);

    // Left face
    indices.emplace_back(4); indices.emplace_back(5); indices.emplace_back(1);
    indices.emplace_back(4); indices.emplace_back(1); indices.emplace_back(0);

    // Right face
    indices.emplace_back(3); indices.emplace_back(2); indices.emplace_back(6);
    indices.emplace_back(3); indices.emplace_back(6); indices.emplace_back(7);

    // Top face
    indices.emplace_back(1); indices.emplace_back(5); indices.emplace_back(6);
    indices.emplace_back(1); indices.emplace_back(6); indices.emplace_back(2);

    // Bottom face
    indices.emplace_back(4); indices.emplace_back(0); indices.emplace_back(3);
    indices.emplace_back(4); indices.emplace_back(3); indices.emplace_back(7);

    CreateMesh();

    //SkyBox�� ���÷�
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;  // ���� ���͸� (Mipmap ��� �� ���� ����)
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;    // ����, ���� ��迡�� Ŭ����
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;    // ����, �Ʒ��� ��迡�� Ŭ����
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;    // ť�� ���� W�൵ Ŭ����
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;   // �� �Լ��� �ʿ����� ���� (�ܼ� ���ø�)
    samplerDesc.BorderColor[0] = 1.0f;                     // ��� ���� (Ŭ������ ������ �� ���)
    samplerDesc.BorderColor[1] = 1.0f;
    samplerDesc.BorderColor[2] = 1.0f;
    samplerDesc.BorderColor[3] = 1.0f;
    samplerDesc.MipLODBias = 0.0f;                         // Mipmap LOD ���̾
    samplerDesc.MaxAnisotropy = 1;                         // ���� ���ø��� ������� ����
    samplerDesc.MinLOD = 0.0f;                             // �ּ� LOD�� 0
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;                // Mipmap�� �ִ� LOD
    materialAsset.SetSamplerState(samplerDesc, 0);

    {
        using namespace std::string_literals;
        std::wstring vertexPath(HLSLManager::EngineShaderPath + L"SkyBoxVS.hlsl"s);
        SetVS(vertexPath.c_str());

        std::wstring pixelPath(HLSLManager::EngineShaderPath + L"SkyBoxPS.hlsl"s);
        SetPS(pixelPath.c_str());
    }
    mainSkyBox = this;
}

void SkyBoxRender::FixedUpdate()
{
}

void SkyBoxRender::Update()
{
}

void SkyBoxRender::LateUpdate()
{
}

void SkyBoxRender::SetSkyBox(TEXTURE_TYPE type, const wchar_t* path)
{
    materialAsset.SetTexture2D(path, type);
    const Texture* brdf_lut = nullptr;
    const Texture* diffuse_ibl = nullptr;
    const Texture* specular_ibl = nullptr;
    const auto& textuers = materialAsset.GetTexturesV2();
    const auto& textuerSlot = materialAsset.GetTexturesSlot();
    size_t textuerCount = textuers.size();
    for (size_t i = 0; i < textuerCount; i++)
    {
        TEXTURE_TYPE type = (TEXTURE_TYPE)textuerSlot[i];
        switch (type)
        {
        case SkyBoxRender::BRDF_LUT:
            brdf_lut = &textuers[i];
            break;
        case SkyBoxRender::Diffuse_IBL:
            diffuse_ibl = &textuers[i];
            break;
        case SkyBoxRender::Specular_IBL:
            specular_ibl = &textuers[i];
            break;
        default:
            continue;
        }
    }

    if(brdf_lut && diffuse_ibl && specular_ibl)
    {
        //ó�� �ʿ�

        //renderer->RemoveBinadble("BRDF_LUT_PS");
        //renderer->AddBinadble("BRDF_LUT_PS", Binadble{ EShaderType::Pixel, EShaderBindable::ShaderResource, 30, (ID3D11ShaderResourceView*)(*brdf_lut)});

        //renderer->RemoveBinadble("BRDF_LUT_CS");
        //renderer->AddBinadble("BRDF_LUT_CS", Binadble{ EShaderType::Compute, EShaderBindable::ShaderResource, 30, (ID3D11ShaderResourceView*)(*brdf_lut) });
      
        //renderer->RemoveBinadble("Diffuse_IBL_PS");
        //renderer->AddBinadble("Diffuse_IBL_PS", Binadble{ EShaderType::Pixel, EShaderBindable::ShaderResource, 31, (ID3D11ShaderResourceView*)(*diffuse_ibl)});
        
         //renderer->RemoveBinadble("Diffuse_IBL_CS");
        //renderer->AddBinadble("Diffuse_IBL_CS", Binadble{ EShaderType::Compute, EShaderBindable::ShaderResource, 31, (ID3D11ShaderResourceView*)(*diffuse_ibl) });

        //renderer->RemoveBinadble("Specular_IBL_PS");
        //renderer->AddBinadble("Specular_IBL_PS", Binadble{ EShaderType::Pixel, EShaderBindable::ShaderResource, 32, (ID3D11ShaderResourceView*)(*specular_ibl)});
       
        //renderer->RemoveBinadble("Specular_IBL_CS");
        //renderer->AddBinadble("Specular_IBL_CS", Binadble{ EShaderType::Compute, EShaderBindable::ShaderResource, 32, (ID3D11ShaderResourceView*)(*specular_ibl) });   
    }
}

void SkyBoxRender::ResetSkyBox(TEXTURE_TYPE type)
{
    materialAsset.ReleaseTexture(type);
}

void SkyBoxRender::CreateMesh()
{
    using namespace Utility;
    if (vertices.empty() || indices.empty())
        return;

    ID3D11Device* pDevcie = RendererUtility::GetDevice();

    meshDrawCommand.meshData.vertexStride = sizeof(Vector4);
    D3D11_BUFFER_DESC bd{};
    bd.ByteWidth = sizeof(Vector4) * vertices.size();
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = vertices.data();
    ComPtr<ID3D11Buffer> vb;
    Check(pDevcie->CreateBuffer(&bd, &vbData, &vb));
    meshDrawCommand.meshData.vertexBuffer.Load(vb);

    meshDrawCommand.meshData.indexCounts = indices.size();
    bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(UINT) * indices.size();
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = indices.data();
    ComPtr<ID3D11Buffer> ib;
    CheckHRESULT(pDevcie->CreateBuffer(&bd, &ibData, &ib));
    meshDrawCommand.meshData.indexBuffer.Load(ib);

    vertices.clear();
    indices.clear();
}
