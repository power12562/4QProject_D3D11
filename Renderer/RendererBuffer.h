#pragma once
#include "RendererCore.h"
#include "ShaderResource.h"
#include <span>

struct RendererBuffer : public ShaderResource
{
public:
	RendererBuffer();
	~RendererBuffer();

public:
	void Load(ComPtr<ID3D11Buffer> buffer);
	void Init(D3D11_BUFFER_DESC bufferDesc, size_t bufferSize, _In_opt_ const void* data = nullptr);
	void Update(const void* data);
	
public:
	operator ID3D11Buffer*() { return buffer.Get(); }

private:
	ComPtr<ID3D11Buffer> buffer{};
	ComPtr<ID3D11DeviceContext> immediateContext{};
};

