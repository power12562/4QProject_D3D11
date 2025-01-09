#pragma once

#include "RendererCore.h"
#include "ShaderResource.h"
#include <string>

struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11ComputeShader;
struct ID3D11InputLayout;

struct ShaderUtillity
{
	static void Compile(const void* data, size_t size, std::string_view shaderModel, ID3DBlob** resultBlob);

};

template<typename T>
struct Shader : public ShaderResource
{
public:
	void Compile(const void* data, size_t size)
	{
		ComPtr<ID3DBlob> shaderBlob = nullptr;
		ShaderUtillity::Compile(data, size, shaderModel, shaderBlob.GetAddressOf());
		
		((T*)this)->CreateShader(shaderBlob.Get());
	}

protected:
	std::string_view shaderModel;
};

struct ShaderHeader : public ShaderResource
{
public:
	ShaderHeader();
	~ShaderHeader();

public:
	void Load(std::string_view newShaderName, const void* data, size_t size);



private:
	ComPtr<ID3DBlob> shaderCode;
	std::shared_ptr<class ShaderInclude> includeInstance;
	std::string shaderName;
};

struct VertexShader : public Shader<VertexShader>
{
public:
	VertexShader();
	~VertexShader();


public:
	operator ID3D11VertexShader* () { return vertexShader.Get(); }
	operator ID3D11InputLayout* () { return inputLayout.Get(); }
	void CreateShader(ID3DBlob* compiledBlob);
	void LoadShader(ID3D11VertexShader* vertexShader, ID3D11InputLayout* inputLayout) 
	{ 
		this->vertexShader = vertexShader; 
		this->inputLayout = inputLayout;
	}

private:
	ComPtr<ID3D11VertexShader> vertexShader;
	ComPtr<ID3D11InputLayout> inputLayout;
};

struct PixelShader : public Shader<PixelShader>
{
public:
	PixelShader();
	~PixelShader();

public:
	bool isForward = false;

public:
	operator ID3D11PixelShader* () { return pixelShader.Get(); }
	void CreateShader(ID3DBlob* compiledBlob);
	void LoadShader(ID3D11PixelShader* pixelShader);



private:
	ComPtr<ID3D11PixelShader> pixelShader{};
};

struct ComputeShader : public Shader<ComputeShader>
{
public:
	ComputeShader();
	~ComputeShader();

public:
	operator ID3D11ComputeShader* () { return computeShader.Get(); }
	void CreateShader(ID3DBlob* compiledBlob);

private:
	ComPtr<ID3D11ComputeShader> computeShader{};
};