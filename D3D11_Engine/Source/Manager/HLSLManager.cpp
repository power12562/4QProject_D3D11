#include "HLSLManager.h"
#include <D3DCore/D3DRenderer.h>
#include <d3d11shader.h>
#include <d3dcompiler.h>
#include <Utility/Console.h>
#include <intrin.h>

HLSLManager& hlslManager = HLSLManager::GetInstance();

using namespace Utility;

HLSLManager::HLSLManager()
{

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
		for (auto& item : sharingInputLayoutMap)
		{
			ULONG refcount = item.second->Release();
			while (refcount != 0)
			{
				refcount = item.second->Release();
			}		
		}
		sharingInputLayoutMap.clear();
	}

	if (!sharingShaderMap.empty())
	{
		for (auto& item : sharingShaderMap)
		{
			ULONG refcount = item.second->Release();
			while (refcount != 0)
			{
				refcount = item.second->Release();
			}
		}
		sharingShaderMap.clear();
	}
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
	CheckHRESULT(d3dRenderer.GetDevice()->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &vertexShader));
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
	CheckHRESULT(d3dRenderer.GetDevice()->CreateInputLayout(inputLayoutDesc.data(), (UINT)inputLayoutDesc.size(),
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), ppOut_InputLayout));

	D3D_SET_OBJECT_NAME(*ppOut_InputLayout, path);

	SafeRelease(pReflector);
	SafeRelease(vertexShaderBuffer);
}

void HLSLManager::MakeShader(const wchar_t* path, ID3D11PixelShader** ppOutput)
{
	ID3D10Blob* pixelShaderBuffer = nullptr;	// 픽셀 셰이더 코드가 저장될 버퍼.
	ID3D11PixelShader* pixelShader = nullptr;	// 픽셀 셰이더가 저장될 곳.
	EXTENSION_TYPE type = ChackShaderFile(path);
	switch (type)
	{
	case HLSLManager::EXTENSION_TYPE::hlsl:
		CheckHRESULT(CompileShaderFromFile(path, "main", PS_MODEL, &pixelShaderBuffer));
		break;
	case HLSLManager::EXTENSION_TYPE::cso:
		CheckHRESULT(LoadShadeFormFile(path, &pixelShaderBuffer));
		break;
	case HLSLManager::EXTENSION_TYPE::null:
		__debugbreak(); //not shader file
		*ppOutput = nullptr;
	}
	CheckHRESULT(d3dRenderer.GetDevice()->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &pixelShader));
	D3D_SET_OBJECT_NAME(pixelShader, path);

	*ppOutput = pixelShader;
	pixelShaderBuffer->Release();
}
