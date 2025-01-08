#pragma once
#include <Core/TSingleton.h>
#include <d3d11.h>
#include <map>
#include <string>

class HLSLManager;
extern HLSLManager& hlslManager;   

class HLSLManager : public TSingleton<HLSLManager>
{
	enum class EXTENSION_TYPE
	{
		hlsl,
		cso,
		null,
	};
	friend class TSingleton;
public:
	static constexpr wchar_t EngineShaderPath[] = L"Resource/EngineShader/";
	static constexpr char VS_MODEL[] = "vs_5_0";
	static constexpr char PS_MODEL[] = "ps_5_0";
public:
	/** 버텍스 셰이더 및 입력 레이아웃 생성.*/
	void CreateSharingShader(const wchar_t* path, ID3D11VertexShader** ppOut_VertexShader, ID3D11InputLayout** ppOut_InputLayout);
	/** 픽셀 셰이더 생성.*/
	void CreateSharingShader(const wchar_t* path, ID3D11PixelShader** ppOutput);

	/*사용중인 Shader 객체들을 Release 합니다. 씬 소멸자에서 호출됩니다.*/
	void ClearSharingShader();

private:
	std::map<std::wstring, ID3D11DeviceChild*> sharingShaderMap;
	std::map<std::wstring, ID3D11InputLayout*> sharingInputLayoutMap;

private:
	EXTENSION_TYPE ChackShaderFile(const wchar_t* extension);

public:
	/** 셰이더를 생성합니다. */
	void MakeShader(const wchar_t* path, ID3D11VertexShader** ppOut_VertexShader, ID3D11InputLayout** ppOut_InputLayout);
	/** 셰이더를 생성합니다. */
	void MakeShader(const wchar_t* path, ID3D11PixelShader** ppOutput);
private:
	HLSLManager();
	~HLSLManager();
};

