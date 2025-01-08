#pragma once
#include <Utility/D3D11Utility.h>
#include <string>
#include <atomic>

namespace E_TEXTURE
{
	//Texture2D albedoTexture : register(t0);
	//Texture2D normalTexture : register(t1);
	//Texture2D specularMap : register(t2);
	//Texture2D emissiveTexture : register(t3);
	//Texture2D opacityTexture : register(t4);
	//Texture2D metalnessTexture : register(t5);
	//Texture2D roughnessTexture : register(t6);
	//Texture2D ambientOcculusionTexture : register(t7);

	//TextureCube Diffuse_IBL_Texture : register(t9);
	//TextureCube Specular_IBL_Texture : register(t10);
	//Texture2D BRDF_LUT : register(t11);

	enum TYPE
	{
		//TEXTURE 동적형.
		Albedo,
		Normal,
		Specular,
		Emissive,
		Opacity,
		Metalness,
		Roughness,
		AmbientOcculusion,
		
		//SkyBox 고정형.
		Diffuse_IBL,
		Specular_IBL,
		BRDF_LUT,
		Null,
	};

	constexpr int BlingPhongTextureCount = Opacity + 1;
	constexpr int PBRTextureCount = AmbientOcculusion + 1;
}

namespace E_TEXTURE_DEFAULT
{
	enum DEFAULT_TEXTURE
	{
		ZERO,
		ONE,
		CUBE_ZERO,
	};
}

class D3DTexture2D
{
	static const size_t MAX_SIZE = 128;
public:
	D3DTexture2D();
	~D3DTexture2D();	

	void resize(size_t newsize);
	inline constexpr size_t size() { return SRVList.size(); }
	inline ID3D11ShaderResourceView** data() { return SRVList.data(); }

	void SetCubeMapTexture(int index, const wchar_t* path);
	void SetTexture2D(int index, const wchar_t* path);
	void SetDefaultTexture(int index, E_TEXTURE_DEFAULT::DEFAULT_TEXTURE type);
	void ResetTexture2D(int index);

	/*SRV를 반환합니다. 없으면 nullptr 반환.*/
	ID3D11ShaderResourceView* operator[](int index);

	/*현재 설정된 path로 다시 로드*/
	void ReloadTexture();
	/**현재 설정된 Path*/
	inline std::wstring& GetPath(int index) { return pathList[index]; }

	virtual void Serialized(std::ofstream& ofs);
	virtual void Deserialized(std::ifstream& ifs);
public:
	D3DTexture2D(const D3DTexture2D& rhs);
	D3DTexture2D& operator=(const D3DTexture2D& rhs);

private:
	std::vector<ID3D11ShaderResourceView*> SRVList{};
	std::vector<std::wstring> pathList{};
};

