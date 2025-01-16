#include "HLSLManager.h"
#include <D3DCore/D3D11_GameApp.h>
#include <Utility/D3D11Utility.h>
#include <d3d11shader.h>
#include <d3dcompiler.h>
#include <Utility/Console.h>
#include <intrin.h>
#include <Renderer.h>
#include <Utility\MemoryUtility.h>
#include <cassert>
#include <fstream>


class ShaderIncludePath : public ID3DInclude
{
public:
	ShaderIncludePath()
	{
		AddPath(std::filesystem::current_path());
	}
	void AddPath(std::filesystem::path fileName)
	{
		//upperbound로 정렬되게 넣기
		auto it = std::upper_bound(paths.begin(), paths.end(), fileName);
		paths.insert(it, fileName);
	}

	void RemovePath(std::filesystem::path fileName)
	{
		//upperbound로 찾기
		auto it = std::upper_bound(paths.begin(), paths.end(), fileName);
		if (it != paths.end())
		{
			paths.erase(it);
		}
	}

	HRESULT Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pByteCount) override
	{
		for (auto& path : paths)
		{
			auto filePath = path.parent_path() / pFileName;
			if (std::filesystem::exists(filePath))
			{
				std::ifstream file(filePath, std::ios::binary);

				if (!file.is_open())
				{
					return E_FAIL;
				}

				file.seekg(0, std::ios::end);
				auto size = file.tellg();
				file.seekg(0, std::ios::beg);

				auto buffer = new char[size];
				file.read(buffer, size);

				*ppData = buffer;
				*pByteCount = size;


				auto it = std::upper_bound(buffers.begin(), buffers.end(), buffer);
				buffers.insert(it, buffer);

				return S_OK;
			}
		}
		return E_FAIL;
	}

	HRESULT Close(LPCVOID pData) override
	{
		auto it = std::upper_bound(buffers.begin(), buffers.end(), pData);
		if (it != buffers.end())
		{
			delete pData;
			buffers.erase(it);
		}
		return S_OK;
	}


private:
	std::vector<std::filesystem::path> paths;
	std::vector<LPCVOID> buffers;
};






HLSLManager& hlslManager = HLSLManager::GetInstance();

using namespace Utility;

HLSLManager::HLSLManager()
{
	includePath = std::make_unique<ShaderIncludePath>();
	includePath->AddPath("Resource/EngineShader/");
}

HLSLManager::~HLSLManager()
{
	if (!sharingShaderMap.empty())
	{
		__debugbreak(); //sharingShaderMap refCounter Error
	}
}

HLSLManager::EXTENSION_TYPE HLSLManager::ChackShaderFile(const wchar_t* extension)
{
	if (wcsstr(extension, L".hlsl"))
	{
		return EXTENSION_TYPE::hlsl;
	}
	else if (wcsstr(extension, L".cso"))
	{
		return EXTENSION_TYPE::cso;
	}
	else
	{
		return EXTENSION_TYPE::null;
	}
}

void HLSLManager::ClearSharingShader()
{
	if (!sharingInputLayoutMap.empty())
	{
		ULONG refcount;
		for (auto& item : sharingInputLayoutMap)
		{
			refcount = item.second->Release();
			//while (refcount != 0)
			//{
			//	refcount = item.second->Release();
			//}
		}
		sharingInputLayoutMap.clear();
	}

	if (!sharingShaderMap.empty())
	{
		ULONG refcount;
		for (auto& item : sharingShaderMap)
		{
			refcount = item.second->Release();
			//while (refcount != 0)
			//{
			//	refcount = item.second->Release();
			//}
		}
		sharingShaderMap.clear();
	}
}

void HLSLManager::AddPath(std::filesystem::path path)
{
	if (!includePath)
	{
		includePath = std::make_unique<ShaderIncludePath>();
	}
	includePath->AddPath(path);
}

void HLSLManager::RemovePath(std::filesystem::path path)
{
	if (!includePath)
	{
		return;
	}
	includePath->RemovePath(path);
}

void HLSLManager::CreateSharingShader(const wchar_t* path, ID3D11VertexShader** ppOut_VertexShader, ID3D11InputLayout** ppOut_InputLayout)
{
	auto findIter = sharingShaderMap.find(path);
	if (findIter != sharingShaderMap.end())
	{
		HRESULT hr = findIter->second->QueryInterface(__uuidof(ID3D11VertexShader), reinterpret_cast<void**>(ppOut_VertexShader));
		if (FAILED(hr))
		{
			*ppOut_VertexShader = nullptr;
			*ppOut_InputLayout = nullptr;
		}
		else
		{
			*ppOut_InputLayout = sharingInputLayoutMap[path];
			(*ppOut_InputLayout)->AddRef();
		}
		return;
	}
	else
	{
		MakeShader(path, ppOut_VertexShader, ppOut_InputLayout);
		(*ppOut_VertexShader)->AddRef();
		(*ppOut_InputLayout)->AddRef();
		sharingShaderMap[path] = *ppOut_VertexShader;
		sharingInputLayoutMap[path] = *ppOut_InputLayout;
	}
}

void HLSLManager::CreateSharingShader(const wchar_t* path, ID3D11PixelShader** ppOutput)
{
	auto findIter = sharingShaderMap.find(path);
	if (findIter != sharingShaderMap.end())
	{
		HRESULT hr = findIter->second->QueryInterface(__uuidof(ID3D11PixelShader), reinterpret_cast<void**>(ppOutput));
		if (FAILED(hr))
		{
			*ppOutput = nullptr;
		}
		return;
	}
	else
	{
		MakeShader(path, ppOutput);
		(*ppOutput)->AddRef();
		sharingShaderMap[path] = *ppOutput;
	}
}

void HLSLManager::CreateSharingShader(const void* data, size_t size, ComPtr<ID3D11PixelShader> ppOutput)
{
	ComPtr< ID3DBlob> blob;
	Utility::CompileShader(includePath.get(), data, size, "main", PS_MODEL, &blob);
	CheckHRESULT(RendererUtility::GetDevice()->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &ppOutput));
}

void HLSLManager::CreateSharingShader(const wchar_t* path, ID3D11ComputeShader** ppOutput)
{
	auto findIter = sharingShaderMap.find(path);
	if (findIter != sharingShaderMap.end())
	{
		HRESULT hr = findIter->second->QueryInterface(__uuidof(ID3D11ComputeShader), reinterpret_cast<void**>(ppOutput));
		if (FAILED(hr))
		{
			*ppOutput = nullptr;
		}
		return;
	}
	else
	{
		MakeShader(path, ppOutput);
		(*ppOutput)->AddRef();
		sharingShaderMap[path] = *ppOutput;
	}
}

void HLSLManager::MakeShader(const wchar_t* path, ID3D11VertexShader** ppOut_VertexShader, ID3D11InputLayout** ppOut_InputLayout)
{
	ID3D10Blob* vertexShaderBuffer = nullptr;	// 정점 셰이더 코드가 저장될 버퍼.
	ID3D11VertexShader* vertexShader = nullptr;	// 정점 셰이더가 저장될 곳.
	EXTENSION_TYPE type = ChackShaderFile(path);
	switch (type)
	{
	case HLSLManager::EXTENSION_TYPE::hlsl:
		CheckHRESULT(CompileShaderFromFile(path, "main", VS_MODEL, &vertexShaderBuffer));
		break;
	case HLSLManager::EXTENSION_TYPE::cso:
		CheckHRESULT(LoadShadeFormFile(path, &vertexShaderBuffer));
		break;
	case HLSLManager::EXTENSION_TYPE::null:
		__debugbreak(); //not shader file
		*ppOut_VertexShader = nullptr;
	}
	CheckHRESULT(RendererUtility::GetDevice()->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &vertexShader));
	*ppOut_VertexShader = vertexShader;
	D3D_SET_OBJECT_NAME(vertexShader, path);

	// 리플렉션을 사용하여 입력 레이아웃 생성
	ID3D11ShaderReflection* pReflector = nullptr;
	CheckHRESULT(D3DReflect(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), IID_PPV_ARGS(&pReflector)));

	D3D11_SHADER_DESC shaderDesc;
	pReflector->GetDesc(&shaderDesc);

	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
	for (UINT i = 0; i < shaderDesc.InputParameters; ++i)
	{
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		pReflector->GetInputParameterDesc(i, &paramDesc);

		// 각 입력 파라미터에 대해 D3D11_INPUT_ELEMENT_DESC 구성
		D3D11_INPUT_ELEMENT_DESC elementDesc = {};
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.Format = GetDXGIFormat(paramDesc.ComponentType, paramDesc.Mask);
		elementDesc.InputSlot = 0;
		elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT; // 자동으로 정렬
		elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		inputLayoutDesc.push_back(elementDesc);
	}
	CheckHRESULT(RendererUtility::GetDevice()->CreateInputLayout(inputLayoutDesc.data(), (UINT)inputLayoutDesc.size(),
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), ppOut_InputLayout));

	D3D_SET_OBJECT_NAME(*ppOut_InputLayout, path);

	SafeRelease(pReflector);
	SafeRelease(vertexShaderBuffer);
}

void HLSLManager::MakeShader(const wchar_t* path, ID3D11PixelShader** ppOutput, size_t* size)
{
	ID3D10Blob* computeShaderBuffer = nullptr;	// 픽셀 셰이더 코드가 저장될 버퍼.
	ID3D11PixelShader* computeShader = nullptr;	// 픽셀 셰이더가 저장될 곳.
	EXTENSION_TYPE type = ChackShaderFile(path);
	switch (type)
	{
	case HLSLManager::EXTENSION_TYPE::hlsl:
		CheckHRESULT(CompileShaderFromFile(path, "main", PS_MODEL, &computeShaderBuffer));
		break;
	case HLSLManager::EXTENSION_TYPE::cso:
		CheckHRESULT(LoadShadeFormFile(path, &computeShaderBuffer));
		break;
	case HLSLManager::EXTENSION_TYPE::null:
		__debugbreak(); //not shader file
		*ppOutput = nullptr;
	}
	CheckHRESULT(RendererUtility::GetDevice()->CreatePixelShader(computeShaderBuffer->GetBufferPointer(), computeShaderBuffer->GetBufferSize(), NULL, &computeShader));
	D3D_SET_OBJECT_NAME(computeShader, path);

	*ppOutput = computeShader;
	computeShaderBuffer->Release();
}

void HLSLManager::MakeShader(const wchar_t* path, ID3D11ComputeShader** ppOutput, size_t* size)
{
	ID3D10Blob* pixelShaderBuffer = nullptr;	// 픽셀 셰이더 코드가 저장될 버퍼.
	ID3D11ComputeShader* pixelShader = nullptr;	// 픽셀 셰이더가 저장될 곳.
	EXTENSION_TYPE type = ChackShaderFile(path);
	switch (type)
	{
	case HLSLManager::EXTENSION_TYPE::hlsl:
		CheckHRESULT(CompileShaderFromFile(path, "main", CS_MODEL, &pixelShaderBuffer));
		break;
	case HLSLManager::EXTENSION_TYPE::cso:
		CheckHRESULT(LoadShadeFormFile(path, &pixelShaderBuffer));
		break;
	case HLSLManager::EXTENSION_TYPE::null:
		__debugbreak(); //not shader file
		*ppOutput = nullptr;
	}
	CheckHRESULT(RendererUtility::GetDevice()->CreateComputeShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &pixelShader));
	D3D_SET_OBJECT_NAME(pixelShader, path);

	*ppOutput = pixelShader;
	pixelShaderBuffer->Release();
}

void ShaderUtility::CopyShader(std::filesystem::path dest, std::filesystem::path sourc)
{
	// EngineShader 폴더 내 모든 파일을 대상으로 복사
	if (!std::filesystem::exists(sourc))
	{
		std::cerr << "엔진 쉐이더 폴더가 없습니다: " << sourc << std::endl;
		__debugbreak();
	}
	if (!std::filesystem::exists(dest))
	{
		std::cerr << "엔진 쉐이더 폴더가 없습니다: " << dest << std::endl;
		__debugbreak();
	}

	for (const auto& entry : std::filesystem::directory_iterator(sourc))
	{
		if (entry.is_regular_file())
		{
			std::filesystem::path destFile = dest / entry.path().filename();

			// destFile이 없거나 수정 시간이 현재 파일보다 과거면 복사
			if (!std::filesystem::exists(destFile) ||
				std::filesystem::last_write_time(destFile) < std::filesystem::last_write_time(entry.path()))
			{
				std::filesystem::copy(entry.path(), destFile, std::filesystem::copy_options::overwrite_existing);
			}

		}
	}
}

void ShaderUtility::CopyShader(std::filesystem::path dest)
{
	std::filesystem::path engineShaderPath = __FILEW__;
	engineShaderPath = engineShaderPath.parent_path().parent_path() / L"EngineShader";

	CopyShader(dest, engineShaderPath);
}
