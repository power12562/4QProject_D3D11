#pragma once
#include <Core/TSingleton.h>
#include <d3d11.h>
#include <map>
#include <string>
#include <vector>
#include <filesystem>
#include <wrl.h>

using namespace Microsoft::WRL;

class HLSLManager;
extern HLSLManager& hlslManager;   

class HLSLManager : public TSingleton<HLSLManager>
{
	enum class EXTENSION_TYPE
	{
		hlsl,
		memory,
		cso,
		null,
	};
	friend class TSingleton;
public:
	static constexpr wchar_t EngineShaderPath[] = L"Resource/EngineShader/";
	static constexpr char VS_MODEL[] = "vs_5_0";
	static constexpr char PS_MODEL[] = "ps_5_0";
	static constexpr char CS_MODEL[] = "cs_5_0";
public:
	/** 버텍스 셰이더 및 입력 레이아웃 생성.*/
	void CreateSharingShader(const wchar_t* path, ID3D11VertexShader** ppOut_VertexShader, ID3D11InputLayout** ppOut_InputLayout);
	/** 픽셀 셰이더 생성.*/
	void CreateSharingShader(const wchar_t* path, ID3D11PixelShader** ppOutput);
	/** 자원 공유를 위해 여기서 하려햇는데 에셋단위로 들고잇으니 효과없음 */
	void CreateSharingShader(const void* data, size_t size, ID3D11PixelShader** ppOutput);
	/** 컴퓨트 셰이더 생성.*/
	void CreateSharingShader(const wchar_t* path, ID3D11ComputeShader** ppOutput);

	/*사용중인 Shader 객체들을 Release 합니다. 씬 소멸자에서 호출됩니다.*/
	void ClearSharingShader();
	void AddPath(std::filesystem::path path);
	void RemovePath(std::filesystem::path path);
private:
	std::map<std::wstring, ID3D11DeviceChild*> sharingShaderMap;
	std::map<std::wstring, ID3D11InputLayout*> sharingInputLayoutMap;
	std::unique_ptr<class ShaderIncludePath> includePath;

private:
	EXTENSION_TYPE ChackShaderFile(const wchar_t* extension);

public:
	/** 셰이더를 생성합니다. */
	void MakeShader(const wchar_t* path, ID3D11VertexShader** ppOut_VertexShader, ID3D11InputLayout** ppOut_InputLayout);
	/** 셰이더를 생성합니다. */
	void MakeShader(const wchar_t* path, ID3D11PixelShader** ppOutput, _In_opt_ size_t* size = nullptr);
	/** 셰이더를 생성합니다. */
	void MakeShader(const wchar_t* path, ID3D11ComputeShader** ppOutput, _In_opt_ size_t* size = nullptr);
private:
	HLSLManager();
	~HLSLManager();
};

struct ShaderUtility
{
	static void CopyShader(std::filesystem::path dest, std::filesystem::path sourc);
	static void CopyShader(std::filesystem::path dest);

};