#pragma once
#include <Utility\D3D11Utility.h>
#include <Core/TSingleton.h>
#include <unordered_map>
#include <string>
#include <minwindef.h>
#include <Material/BlingPhongMaterial.h>
#include <D3DCore/D3DTexture2D.h>

class TextureManager;
extern TextureManager& textureManager;

class TextureManager : public TSingleton<TextureManager>
{
private:
	friend class TSingleton;
	TextureManager();
	~TextureManager();

public:
	/**파일 읽고 텍스쳐 등록.*/
	ULONG CreateSharingTexture(const wchar_t* path, ID3D11ShaderResourceView** ppOut_ResourceView);
	/**텍스쳐 압축 후 등록.*/
	ULONG CreateSharingCompressTexture(const wchar_t* path, ID3D11ShaderResourceView** ppOut_ResourceView, Utility::E_COMPRESS::TYPE type);
	/**큐브맵 텍스쳐 등록. dds 큐브맵만 사용 가능*/
	ULONG CreateSharingCubeMap(const wchar_t* path, ID3D11ShaderResourceView** ppOut_ResourceView);
	/** 텍스쳐 반납.*/
	ULONG ReleaseSharingTexture(const wchar_t* path);

	/** 텍스쳐 존재 여부 반환. 있으면 CreateSharing*** 함수로 얻을수 있다.*/
	bool IsTextureLoaded(const wchar_t* path);
	/** 매니저를 거치지 않는 텍스쳐를 등록.*/
	void InsertTexture(const wchar_t* key, ID3D11ShaderResourceView* pSRV);
	
public:
	ID3D11ShaderResourceView* GetDefaultTexture(E_TEXTURE_DEFAULT::DEFAULT_TEXTURE texture);
	void ReleaseDefaultTexture();

private:
	ID3D11ShaderResourceView* GetOneTexture();
	ID3D11ShaderResourceView* GetZeroTexture();
	ID3D11ShaderResourceView* GetCubeZeroTexture();

private:
	std::unordered_map<std::wstring, ID3D11ShaderResourceView*> resourceMap;

	void CreateDefaultTexture(const float (&pixel)[4], ID3D11ShaderResourceView** ppSRV);
	void CreateDefaultCubeTexture(const float (&pixel)[4], ID3D11ShaderResourceView** ppSRV);
	ID3D11ShaderResourceView* oneTexture = nullptr;
	ID3D11ShaderResourceView* zeroTexture = nullptr;
	ID3D11ShaderResourceView* cubeZeroTexture = nullptr;
};


