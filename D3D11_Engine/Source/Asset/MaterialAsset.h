#pragma once
#include <Asset/Base/AssetBase.h>
#include <DrawCommand.h>

enum class ETextureType 
{
	Albedo,
	Normal,
	Specular,
	Emissive,
	Opacity,
	Metalness,
	Roughness,
	AmbientOcculusion,
	Null
};

class MaterialAsset : public AssetBase
{
public:
	MaterialAsset();
	virtual ~MaterialAsset() override;

public:
	void CopyAsset(const MaterialAsset& rhs);

	void ReleaseSamplerState(uint32_t slot);
	void SetSamplerState(const D3D11_SAMPLER_DESC& desc, uint32_t slot);

public:
	void SetPixelShader(std::string shaderCode, bool isForward);

	void SetTexture2D(const wchar_t* path, uint32_t slot);
	template <typename T> void SetTexture2D(const wchar_t* path, T type)	
	{
		uint32_t slot = static_cast<uint32_t>(type);
		SetTexture2D(path, slot);
	}

	void ReleaseTexture(uint32_t slot);
	template <typename T> void ReleaseTexture(T type)
	{
		uint32_t slot = static_cast<uint32_t>(type);
		ReleaseTexture(slot);
	}
protected:
	virtual void Serialized(std::ofstream& ofs) override;
	virtual void Deserialized(std::ifstream& ifs) override;

private:
	std::string				pixelShaderData;
	PixelShader					pixelShader;
	std::vector<uint32_t>		texturesSlot;
	std::vector<Texture>		texturesV2;
	std::vector<std::wstring>	currTexturePath;

	std::vector<uint32_t>		samplerSlot;
	std::vector<SamplerState>	samplers;
	bool isForward;
public:
	const PixelShader& GetPS() { return pixelShader; }
	const std::vector<uint32_t>& GetTexturesSlot() { return texturesSlot; }
	const std::vector<Texture>& GetTexturesV2() { return texturesV2; }
	const std::vector<std::wstring>& GetCurrTexturesPath() { return currTexturePath; }

	const std::vector<uint32_t>& GetSamplerSlot() { return samplerSlot; }
	const std::vector<SamplerState>& GetSamplers() { return samplers; }
};