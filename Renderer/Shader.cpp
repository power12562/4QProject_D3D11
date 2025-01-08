#include "Shader.h"
#include <d3d11.h>
#include <d3dcompiler.h>

void ShaderUtillity::Compile(const void* data, size_t size, std::string_view shaderModel, ID3DBlob** resultBlob)
{
	HRESULT result;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	result = D3DCompile(data,
						size,
						nullptr,
						nullptr,
						nullptr,
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
