#include "MeshRender.h"
#include <Manager/ResourceManager.h>
#include <Manager/HLSLManager.h>
#include <Manager/TextureManager.h>
#include <Utility/MemoryUtility.h>
#include <ranges>
#include <Material/PBRMaterial.h>
#include <format>

using namespace Utility;

void MeshRender::ReloadShaderAll()
{
	hlslManager.ClearSharingShader();
	for (auto& item : instanceList)
	{
		item->SetVS(item->currVSpath.c_str());
		item->SetPS(item->currPSpath.c_str());
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
	//�ʱ�ȭ
	meshDrawCommand.meshData.shaderResources.clear();
	meshDrawCommand.materialData.shaderResources.clear();

	//�׽�Ʈ��
	for (size_t i = 0; i < (size_t)ETextureType::Null; i++)
	{
		ETextureType type = (ETextureType)i;
		Binadble binadble{};
		binadble.shaderType   = EShaderType::Pixel;
		binadble.bindableType = EShaderBindable::ShaderResource;
		binadble.slot = i;
		switch (type)
		{
		case ETextureType::Albedo:
			binadble.bind = textureManager.GetDefaultTexture(DEFAULT_TEXTURE::ONE);
			break;
		case ETextureType::Normal:
			binadble.bind = textureManager.GetDefaultTexture(DEFAULT_TEXTURE::ZERO);
			break;
		case ETextureType::Specular:
			binadble.bind = textureManager.GetDefaultTexture(DEFAULT_TEXTURE::ONE);
			break;
		case ETextureType::Emissive:
			binadble.bind = textureManager.GetDefaultTexture(DEFAULT_TEXTURE::ZERO);
			break;
		case ETextureType::Opacity:
			binadble.bind = textureManager.GetDefaultTexture(DEFAULT_TEXTURE::ONE);
			break;
		case ETextureType::Metalness:
			binadble.bind = textureManager.GetDefaultTexture(DEFAULT_TEXTURE::ZERO);
			break;
		case ETextureType::Roughness:
			binadble.bind = textureManager.GetDefaultTexture(DEFAULT_TEXTURE::ZERO);
			break;
		case ETextureType::AmbientOcculusion:
			binadble.bind = textureManager.GetDefaultTexture(DEFAULT_TEXTURE::ONE);
			break;
		case ETextureType::Null:
			break;
		default:
			break;
		}
		meshDrawCommand.materialData.shaderResources.push_back(binadble);
	}

	//TransformBufferData ������Ʈ(��Ƽ üũ �ʿ��ҵ�?)
	transformBuffer.Set(
		TransformBufferData
		{
			.World = XMMatrixTranspose(transform.GetWM()),
			.WorldInverseTranspose = transform.GetIWM()
		}
	);

	//���
	meshDrawCommand.meshData.shaderResources.push_back(
		Binadble
		{
			.shaderType = EShaderType::Vertex,
			.bindableType = EShaderBindable::ConstantBuffer,
			.slot = 0,
			.bind = (ID3D11Buffer*)transformBuffer
		}
	);

	//�ؽ��� ���
	size_t textureCount = materialAsset.GetTexturesV2().size();
	const auto& textures = materialAsset.GetTexturesV2();
	const auto& textureSlot = materialAsset.GetTexturesSlot();
	for (size_t i = 0; i < textureCount; i++)
	{
		Binadble bind{};
		bind.bindableType = EShaderBindable::ShaderResource;
		bind.shaderType = EShaderType::Pixel;
		bind.slot = textureSlot[i];
		bind.bind = (ID3D11ShaderResourceView*)textures[i];
		meshDrawCommand.materialData.shaderResources.push_back(bind);
	}

	//���÷� ���
	size_t samplersCount = materialAsset.GetSamplers().size();
	const auto& samplers = materialAsset.GetSamplers();
	const auto& samplerSlot = materialAsset.GetSamplerSlot();
	for (size_t i = 0; i < samplersCount; i++)
	{
		Binadble bind{};
		bind.bindableType = EShaderBindable::Sampler;
		bind.shaderType = EShaderType::Pixel;
		bind.slot = samplerSlot[i];
		bind.bind = (ID3D11SamplerState*)samplers[i];
		meshDrawCommand.materialData.shaderResources.push_back(bind);
	}

	//������ ����
	meshDrawCommand.materialData.pixelShader.isForward = this->isForward;

	//�ٿ�� �ڽ�
	meshDrawCommand.meshData.boundingBox = gameObject.GetOBBToWorld();

	//������Ʈ ȣ��
	UpdateMeshDrawCommand();

	//��ο� ��û
	D3D11_GameApp::GetRenderer().AddDrawCommand(meshDrawCommand);
}

void MeshRender::SetVS(const wchar_t* path)
{
	currVSpath = path;
	{
		ComPtr<ID3D11VertexShader> vs;
		ComPtr<ID3D11InputLayout> il;
		hlslManager.CreateSharingShader(currVSpath.c_str(), &vs, &il);
		meshDrawCommand.meshData.vertexShader.LoadShader(vs.Get(), il.Get());
	}
}

void MeshRender::SetPS(const wchar_t* path)
{
	currPSpath = path;
	{
		ComPtr<ID3D11PixelShader> ps;
		hlslManager.CreateSharingShader(currPSpath.c_str(), &ps);
		meshDrawCommand.materialData.pixelShader.LoadShader(ps.Get());
	}
}

void MeshRender::SetMeshResource(int meshID)
{
	if (meshID < 0)
	{
		__debugbreak();
		return;
	}
	else
	{
		MeshID = meshID;
	}

	GameObject* MeshObject = GameObject::Find(gameObject.Name.c_str());
	if (MeshObject)
	{
		for (size_t i = 0; i < MeshObject->GetComponentCount(); i++)
		{
			if (MeshRender* renderer = MeshObject->GetComponentAtIndex<MeshRender>(i))
			{
				if (renderer->MeshID == MeshID)
				{
					meshDrawCommand = renderer->meshDrawCommand;
					materialAsset.CopyAsset(renderer->materialAsset);
					SetVS(renderer->currVSpath.c_str());
					SetPS(renderer->currPSpath.c_str());
				}
			}
		}
	}
}

