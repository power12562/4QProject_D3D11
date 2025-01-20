#include "MaterialAsset.h"
#include <framework.h>
#include <Utility/SerializedUtility.h>

MaterialAsset::MaterialAsset()
	: AssetBase(L"MaterialAsset")
{

}

MaterialAsset::~MaterialAsset()
{
	texturesV2.clear();
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

void MaterialAsset::SetPixelShader(std::string shaderCode, bool isForward)
{
	this->isForward = isForward;
	pixelShaderData = shaderCode;

	ComPtr<ID3D11PixelShader> rhiPixelShader;
	hlslManager.CreateSharingShader(shaderCode.data(), shaderCode.size(), &rhiPixelShader);
	pixelShader.LoadShader(rhiPixelShader.Get());
	pixelShader.isForward = isForward;
}

void MaterialAsset::SetCubeMapTexture(const wchar_t* path, uint32_t slot)
{
	ReleaseTexture(slot);
	currTexturePath.emplace_back(path);
	texturesSlot.emplace_back(slot);
	texturesV2.emplace_back();

	Texture& textures = texturesV2.back();
	ComPtr<ID3D11ShaderResourceView> textuer2D;
	textureManager.CreateSharingCubeMap(path, &textuer2D);
	textures.LoadTexture(textuer2D.Get());
	textuer2D->AddRef();
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
	textuer2D->AddRef();
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
		texturesV2.erase(texturesV2.begin() + index);
		textureManager.ReleaseSharingTexture(path->c_str());
		currTexturePath.erase(currTexturePath.begin() + index);
		texturesSlot.erase(texturesSlot.begin() + index);
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
	Write::data(ofs, isForward);
	
}

void MaterialAsset::Deserialized(std::ifstream& ifs)
{
	using namespace Binary;
	texturesV2.clear();
	for (size_t i = 0; i < currTexturePath.size(); i++)
	{
		if (currTexturePath[i].size())
		{
			textureManager.ReleaseSharingTexture(currTexturePath[i].c_str());
		}
	}
	currTexturePath.clear();
	texturesSlot.clear();
	std::vector<uint32_t>		tempTexturesSlot;
	std::vector<std::wstring>	tempCurrTexturePath;

	tempTexturesSlot.resize(Read::data<size_t>(ifs));
	Read::std_vector(ifs, tempTexturesSlot);
	auto size = Read::data<size_t>(ifs);

	tempCurrTexturePath.resize(Read::data<size_t>(ifs));
	for (size_t i = 0; i < tempCurrTexturePath.size(); i++)
	{
		tempCurrTexturePath[i] = Read::wstring(ifs);
	}
	
	for (size_t i = 0; i < tempCurrTexturePath.size(); i++)
	{
		SetTexture2D(tempCurrTexturePath[i].c_str(), tempTexturesSlot[i]);
	}
	pixelShaderData = Read::string(ifs );
	isForward = Read::data<bool>(ifs);
	SetPixelShader(pixelShaderData, isForward);
}
