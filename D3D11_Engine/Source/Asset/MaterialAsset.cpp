#include "MaterialAsset.h"
#include <framework.h>
#include <Utility/SerializedUtility.h>

MaterialAsset::MaterialAsset()
	: AssetBase(L"MaterialAsset")
{

}

MaterialAsset::~MaterialAsset()
{
	for (auto& path : currTexturePath)
	{
		textureManager.ReleaseSharingTexture(path.c_str());
	}
}

void MaterialAsset::CopyAsset(const MaterialAsset& rhs)
{
	if (this == &rhs)
		return;

	texturesSlot = rhs.texturesSlot;
	texturesV2 = rhs.texturesV2;
	currTexturePath = rhs.currTexturePath;

	samplerSlot = rhs.samplerSlot;
	samplers = rhs.samplers;
}

void MaterialAsset::SetSamplerState(const D3D11_SAMPLER_DESC& desc, uint32_t slot)
{
	ReleaseSamplerState(slot);
	samplerSlot.emplace_back(slot);
	samplers.emplace_back();
	SamplerState& sampler = samplers.back();
	sampler.SetSampler(desc);
}

void MaterialAsset::ReleaseSamplerState(uint32_t slot)
{
	for (size_t i = 0; i < samplerSlot.size(); i++)
	{
		if (samplerSlot[i] == slot)
		{
			samplers.erase(samplers.begin() + i);
			samplerSlot.erase(samplerSlot.begin() + i);
			break;
		}
	}
}

void MaterialAsset::SetPixelShader(std::string shaderCode)
{
	pixelShaderData = shaderCode;

	ComPtr<ID3D11PixelShader> rhiPixelShader;
	hlslManager.CreateSharingShader(shaderCode.data(), shaderCode.size(), &rhiPixelShader);
	pixelShader.LoadShader(rhiPixelShader.Get());
	///////////////////////////////////////////////////////////////////
	// �𸣰ٴ�
}

void MaterialAsset::SetTexture2D(const wchar_t* path, uint32_t slot)
{
	ReleaseTexture(slot);
	currTexturePath.emplace_back(path);
	texturesSlot.emplace_back(slot);
	texturesV2.emplace_back();
	Texture& textures = texturesV2.back();
	ComPtr<ID3D11ShaderResourceView> textuer2D;
	textureManager.CreateSharingTexture(path, &textuer2D);
	textures.LoadTexture(textuer2D.Get());
}

void MaterialAsset::ReleaseTexture(uint32_t slot)
{
	std::wstring* path = nullptr;
	size_t index = 0;
	for (size_t i = 0; i < texturesSlot.size(); i++)
	{
		if (texturesSlot[i] == slot)
		{
			path = &currTexturePath[i];
			index = i;
			break;
		}
	}
	if (path)
	{
		textureManager.ReleaseSharingTexture(path->c_str());
		currTexturePath.erase(currTexturePath.begin() + index);
		texturesSlot.erase(texturesSlot.begin() + index);
		texturesV2.erase(texturesV2.begin() + index);
	}
}

void MaterialAsset::Serialized(std::ofstream& ofs)
{
	using namespace Binary;

	size_t slotSize = texturesSlot.size();
	Write::data(ofs, slotSize);
	Write::std_vector(ofs, texturesSlot);

	size_t texturesSize = texturesV2.size();
	Write::data(ofs, texturesSize);


	size_t pathSize = currTexturePath.size();
	Write::data(ofs, pathSize);
	for (auto& path : currTexturePath)
	{
		Write::wstring(ofs, path);
	}
	Write::string(ofs, pixelShaderData);
}

void MaterialAsset::Deserialized(std::ifstream& ifs)
{
	using namespace Binary;

	texturesSlot.resize(Read::data<size_t>(ifs));
	Read::std_vector(ifs, texturesSlot);

	texturesV2.resize(Read::data<size_t>(ifs));


	currTexturePath.resize(Read::data<size_t>(ifs));
	for (size_t i = 0; i < currTexturePath.size(); i++)
	{
		if (currTexturePath[i].size())
		{
			textureManager.ReleaseSharingTexture(currTexturePath[i].c_str());
		}
		currTexturePath[i] = Read::wstring(ifs);
	}

	for (size_t i = 0; i < currTexturePath.size(); i++)
	{
		ComPtr<ID3D11ShaderResourceView> textuer2D;
		textureManager.CreateSharingTexture(currTexturePath[i].c_str(), &textuer2D);
		texturesV2[i].LoadTexture(textuer2D.Get());
		textuer2D->AddRef();
	}
	pixelShaderData = Read::string(ifs );
	SetPixelShader(pixelShaderData);
}
