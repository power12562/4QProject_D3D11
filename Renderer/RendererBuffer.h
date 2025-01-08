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
	void Init(D3D11_BUFFER_DESC bufferDesc, size_t bufferSize, _In_opt_ void* data = nullptr);
	void Update(void* data);
	void Bind();
	
public:
	ID3D11Buffer* GetBuffer() { return buffer.Get(); }

private:
	ComPtr<ID3D11Buffer> buffer{};
	ComPtr<ID3D11DeviceContext> immediateContext{};
	uint32_t bufferSize;
};
