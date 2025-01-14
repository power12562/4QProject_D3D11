#include "Shader.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <map>



class ShaderInclude : public ID3DInclude
{
public:
	void Push(std::string_view fileName, ComPtr<ID3DBlob> blob)
	{
		m_blobs[fileName.data()] = blob;
	}

	void Pop(std::string_view fileName)
	{
		if (m_blobs.find(fileName.data()) != m_blobs.end())
		{
			m_blobs.erase(fileName.data());
		}
	}

	ID3DBlob* find(std::string_view fileName)
	{
		auto it = m_blobs.find(fileName.data());
		if (it == m_blobs.end())
		{
			return nullptr;
		}
		else
		{
			return it->second.Get();
		}

	}

	HRESULT Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pByteCount) override
	{
		auto it = m_blobs.find(pFileName);
		if (it != m_blobs.end())
		{
			*ppData = it->second->GetBufferPointer();
			*pByteCount = it->second->GetBufferSize();
			return S_OK;
		}
		return E_FAIL;
	}

	HRESULT Close(LPCVOID pData) override
	{
		// No need to close the blobs in this simple case
		return S_OK;
	}


	static std::shared_ptr<ShaderInclude> GetInstance()
	{
		static std::weak_ptr<ShaderInclude> instance;
		std::shared_ptr<ShaderInclude> m_instance; 
		m_instance = instance.lock();
		if (!m_instance)
		{
			m_instance = std::make_shared<ShaderInclude>();
			instance = m_instance;
		}

	
		return m_instance;
	}

private:
	std::map<std::string, ComPtr<ID3DBlob>> m_blobs;
};




void ShaderUtillity::Compile(const void* data, size_t size, std::string_view shaderModel, ID3DBlob** resultBlob)
{
	HRESULT result;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	auto shaderInclude = ShaderInclude::GetInstance();
	result = D3DCompile(data,
						size,
						nullptr,
						nullptr,
						shaderInclude.get(),
						"main",
						shaderModel.data(),
						0,
						0,
						resultBlob,
						&errorBlob);

	if (FAILED(result) && errorBlob)
	{
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}

}

ShaderHeader::ShaderHeader() = default;
ShaderHeader::~ShaderHeader()
{
	includeInstance->Pop(shaderName);
}

void ShaderHeader::Load(std::string_view newShaderName, const void* data, size_t size)
{
	shaderName = newShaderName;

	D3DCreateBlob(size, shaderCode.GetAddressOf());
	memcpy(shaderCode->GetBufferPointer(), data, size);

	includeInstance = ShaderInclude::GetInstance();
	includeInstance->Push(shaderName, shaderCode);

}




VertexShader::VertexShader()
{
	shaderModel = "vs_5_0";
}
VertexShader::~VertexShader() = default;

void VertexShader::CreateShader(ID3DBlob* compiledBlob)
{
	auto device = RendererUtility::GetDevice();
	device->CreateVertexShader(compiledBlob->GetBufferPointer(), 
							   compiledBlob->GetBufferSize(),
							   nullptr, 
							   vertexShader.GetAddressOf());
	DebugBreak();
}


PixelShader::PixelShader()
{
	shaderModel = "ps_5_0";
}
PixelShader::~PixelShader() = default;

void PixelShader::CreateShader(ID3DBlob* compiledBlob)
{
	auto device = RendererUtility::GetDevice();
	device->CreatePixelShader(compiledBlob->GetBufferPointer(),
							  compiledBlob->GetBufferSize(),
							  nullptr,
							  pixelShader.GetAddressOf());
}

ComputeShader::ComputeShader()
{
	shaderModel = "cs_5_0";
}
ComputeShader::~ComputeShader() = default;

void ComputeShader::CreateShader(ID3DBlob* compiledBlob)
{
	auto device = RendererUtility::GetDevice();
	device->CreateComputeShader(compiledBlob->GetBufferPointer(),
								compiledBlob->GetBufferSize(),
								nullptr,
								computeShader.GetAddressOf());
}


